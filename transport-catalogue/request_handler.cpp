#include "request_handler.h"
#include <map>


RequestHandler::RequestHandler(catalogue::TransportCatalogue& db, renderer::MapRenderer& renderer, std::ostream& output)
	:db_(db), renderer_(renderer), output(output)
{
}

void RequestHandler::ExecuteCommands(const commands::Commands& commands) {
	RoutingCommandsLoad(commands.routing_settings);
	RenderCommandsLoad(commands.render_settings);
	WriteCommandsLoad(commands.write_commands);
	ReadCommandsLoad(commands.read_commands);
}

void RequestHandler::RoutingCommandsLoad(const commands::Commands::RoutingCommands& commands)
{
	transport_router.SetSettings(commands.router_settings_comands);
}

void RequestHandler::RenderCommandsLoad(const commands::Commands::RenderCommands& commands)
{
	if (commands.is_load) {
		renderer_.SetSettings(commands.render_settings_comands);
	}
}

void RequestHandler::WriteCommandsLoad(const commands::Commands::WriteCommands& commands)
{
	if (commands.is_load) {
		for (const auto& stop_distance : commands.stops) {
			db_.AddStop(stop_distance.stop);
			for (const auto& distance : stop_distance.distances) {
				db_.SetDistance(stop_distance.stop.name, distance.destination_stop, distance.value);
			}
		}
		for (const auto& bus_command : commands.buses) {
			catalogue::parse_structs::Bus bus;
			bus.name = std::move(bus_command.name);
			bus.is_roundtrip = bus_command.is_roundtrip;
			bus.is_reverse = !bus.is_roundtrip;

			for (const auto& stop : bus_command.stops) {
				bus.stops.push_back(&db_.GetStop(stop));
			}

			if (bus.stops.size() != 0) {
				for (size_t i = 1; i < bus.stops.size(); ++i) {
					bus.route_length += db_.GetDistance(bus.stops.at(i - 1)->name, bus.stops.at(i)->name);
					bus.direct_length += geo::ComputeDistance(bus.stops.at(i - 1)->coords, bus.stops.at(i)->coords);
				}

				if (bus.is_reverse) {
					for (size_t i = bus.stops.size() - 1; i > 0; --i) {
						bus.route_length += db_.GetDistance(bus.stops.at(i)->name, bus.stops.at(i - 1)->name);
						bus.direct_length += geo::ComputeDistance(bus.stops.at(i)->coords, bus.stops.at(i - 1)->coords);
					}
				}
			}
			db_.AddBus(bus);
		}

		//Grapth filling
		transport_router.ChangeSize(commands.stops.size()); //Init transport router
		auto& bus_list = db_.GetBusList();
		for (const auto& [bus_name, bus] : bus_list) {
			
			if (bus->stops.size() != 0) {
				//Form stop list
				std::vector<const catalogue::parse_structs::Stop*> stop_list;
				for (int i = 0; i < bus->stops.size(); ++i) {
					stop_list.push_back(bus->stops.at(i));
				}
				if (bus->is_reverse) {
					for (int i = bus->stops.size() - 1; i >= 0; --i) {
						stop_list.push_back(bus->stops.at(i));
					}
				}

				//Fill graph
				for (size_t i = 0; i < stop_list.size(); ++i) {
					double distance = 0;
					for (size_t j = i + 1; j < stop_list.size(); ++j) {
						distance += db_.GetDistance(stop_list.at(j - 1)->name, stop_list.at(j)->name);
						double time = distance / (double)1000 / transport_router.bus_velocity * 60;
						time += transport_router.bus_wait_time;
						transport_router.AddStopEnge(stop_list.at(i), stop_list.at(j), time, bus->name, j - i);
					}
				}
			}
		}
	}
}

void RequestHandler::ReadCommandsLoad(const commands::Commands::ReadCommands& commands)
{
	if (commands.is_load) {
		json::Array arr;
		auto router = transport_router.GetRouter();

		for (const auto& command : commands.commands) {
			if (command.index() == 0) {
				auto map_command = std::get<commands::ReadMapCommandInfo>(command);
				FormMapCommand(arr, map_command);
			}

			if (command.index() == 1) {
				auto stop_command = std::get<commands::ReadStopCommandInfo>(command);
				FormReadStopCommand(arr, stop_command);
			}
			if (command.index() == 2) {
				auto bus_command = std::get<commands::ReadBusCommandInfo>(command);
				FormReadBusCommand(arr, bus_command);
			}
			if (command.index() == 3) {
				auto route_command = std::get<commands::ReadRouteCommandInfo>(command);
				FormRouteCommand(arr, route_command, router);
			}
		}
		if (arr.size() > 0) {
			json::Node result(arr);
			json::Print(json::Document(result), output);
		}
	}
}

void RequestHandler::FormMapCommand(json::Array& arr, const commands::ReadMapCommandInfo& command)
{
	std::stringstream ss;
	renderer_.FormMap(db_.GetBusList(), ss);
	std::string map_string = ss.str();

	arr.emplace_back(std::map<std::string, json::Node>{
		{ "map", map_string},
		{ "request_id", command.request_id }
	});
}

void RequestHandler::FormReadStopCommand(json::Array& arr, const commands::ReadStopCommandInfo& command)
{
	auto info = db_.GetStopInfo(command.stop_name);
	if (info.is_found) {
		json::Array bus_names;
		for (const catalogue::parse_structs::Bus* bus : info.buses) {
			bus_names.emplace_back(bus->name);
		}
		arr.emplace_back(std::map<std::string, json::Node>{
			{"buses", bus_names},
			{ "request_id", command.request_id }
		});
	}
	else {
		if (info.is_exist) {
			arr.emplace_back(std::map<std::string, json::Node>{
				{ "request_id", command.request_id },
				{ "buses", json::Array{} }
			});
		}
		else {
			arr.emplace_back(std::map<std::string, json::Node>{
				{ "request_id", command.request_id },
				{ "error_message", (std::string)"not found" }
			});
		}
	}
}

void RequestHandler::FormReadBusCommand(json::Array& arr, const commands::ReadBusCommandInfo& command)
{
	auto info = db_.GetBusInfo(command.bus_name);
	if (info.is_found) {
		arr.emplace_back(std::map<std::string, json::Node>{
			{"curvature", info.route_length / info.direct_length},
			{ "request_id", command.request_id },
			{ "route_length", info.route_length },
			{ "stop_count", info.stops },
			{ "unique_stop_count", info.unique_stops }
		});
	}
	else {
		arr.emplace_back(std::map<std::string, json::Node>{
			{ "request_id", command.request_id },
			{ "error_message", (std::string)"not found" }
		});
	}
}

void RequestHandler::FormRouteCommand(json::Array& arr, const commands::ReadRouteCommandInfo& command, const graph::Router<router::TransportRouter::Type>& router)
{
	auto info = transport_router.FindWay(&db_.GetStop(command.from), &db_.GetStop(command.to), router);
	if (info) {
		json::Array items;
		double total_time = 0;
		for (const graph::EdgeId edge : info.value().edges) {
			auto edge_value = transport_router.GetEdge(edge);
			auto stop1 = db_.GetStopById(edge_value.from);
			auto stop2 = db_.GetStopById(edge_value.to);
			json::Dict item_wait = {
				{"stop_name", stop1.name},
				{ "time" , transport_router.bus_wait_time},
				{ "type" , (std::string)"Wait"}
			};
			items.push_back(std::move(item_wait));
			total_time += transport_router.bus_wait_time;

			double bus_time = edge_value.weight - (double)transport_router.bus_wait_time;
			json::Dict item_bus = {
				{
					{"bus", edge_value.name},
					{"span_count", edge_value.span_count},
					{"time",  bus_time },
					{"type", (std::string)"Bus"}
			  },
			};
			total_time += bus_time;
			items.push_back(std::move(item_bus));
		}

		arr.emplace_back(std::map<std::string, json::Node>{
			{ "request_id", command.request_id },
			{ "total_time", total_time },
			{ "items", items }
		});
	}
	else {
		arr.emplace_back(std::map<std::string, json::Node>{
			{ "request_id", command.request_id },
			{ "error_message", (std::string)"not found" }
		});
	}
}


std::optional<catalogue::parse_structs::BusInfo> RequestHandler::GetBusStat(const std::string_view& bus_name) const
{
	return db_.GetBusInfo(bus_name);
}

const std::vector<const catalogue::parse_structs::Bus*> RequestHandler::GetBusesByStop(const std::string_view& stop_name) const
{
	return db_.GetStopInfo(stop_name).buses;
}

const std::vector<const catalogue::parse_structs::Bus*> RequestHandler::GetIntersectingBusesByStop(const std::string_view& stop_name_1, const std::string_view& stop_name_2) const
{
	//Sorted by name
	auto bus_list_1 = db_.GetStopInfo(stop_name_1).buses;
	auto bus_list_2 = db_.GetStopInfo(stop_name_2).buses;
	std::vector<const catalogue::parse_structs::Bus*> result;
	std::set_intersection(bus_list_1.begin(), bus_list_1.end(), bus_list_2.begin(), bus_list_2.end(), std::back_inserter(result));
	return result;
}
