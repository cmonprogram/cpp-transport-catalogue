#include "request_handler.h"
#include <map>


RequestHandler::RequestHandler(catalogue::TransportCatalogue& db, renderer::MapRenderer& renderer, std::ostream& output)
	:db_(db), renderer_(renderer), output(output)
{
}

void RequestHandler::ExecuteCommands(commands::Commands commands) {
	if (commands.render_settings.render_settings_load) {
		renderer_.SetSettings(commands.render_settings.render_settings_comands);
	}

	if (commands.write_commands.write_command_load) {
		for (const auto& stop_distance : commands.write_commands.stops) {
			db_.AddStop(stop_distance.stop);
			for (const auto& distance : stop_distance.distances) {
				db_.SetDistance(stop_distance.stop.name, distance.destination_stop, distance.value);
			}
		}
		for (const auto& bus_command : commands.write_commands.buses) {
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
	}


	if (commands.read_commands.read_command_load) {
		json::Array arr;

		for (const auto& command : commands.read_commands.commands) {
			if (command.index() == 0) {
				auto map_command = std::get<commands::ReadMapCommandInfo>(command);
				std::stringstream ss;
				renderer_.FormMap(db_.GetBusList(), ss);
				std::string map_string = ss.str();

				arr.emplace_back(std::map<std::string, json::Node>{
					{ "map", map_string},
					{ "request_id", map_command.request_id }
				});
			}
			if (command.index() == 1) {
				auto stop_command = std::get<commands::ReadStopCommandInfo>(command);
					auto info = db_.GetStopInfo(stop_command.stop_name);
					if (info.is_found) {
						json::Array bus_names;
						for (const catalogue::parse_structs::Bus* bus : info.buses) {
							bus_names.emplace_back(bus->name);
						}
						arr.emplace_back(std::map<std::string, json::Node>{
							{"buses", bus_names},
							{ "request_id", stop_command.request_id }
						});
					}
					else {
						if (info.is_exist) {
							arr.emplace_back(std::map<std::string, json::Node>{
								{ "request_id", stop_command.request_id },
								{ "buses", json::Array{} }
							});
						}
						else {
							arr.emplace_back(std::map<std::string, json::Node>{
								{ "request_id", stop_command.request_id },
								{ "error_message", (std::string)"not found" }
							});
						}

					}
				
			}
			if (command.index() == 2) {
				auto bus_command = std::get<commands::ReadBusCommandInfo>(command);
				auto info = db_.GetBusInfo(bus_command.bus_name);
				if (info.is_found) {
					arr.emplace_back(std::map<std::string, json::Node>{
						{"curvature", info.route_length / info.direct_length},
						{ "request_id", bus_command.request_id },
						{ "route_length", info.route_length },
						{ "stop_count", info.stops },
						{ "unique_stop_count", info.unique_stops }
					});
				}
				else {
					arr.emplace_back(std::map<std::string, json::Node>{
						{ "request_id", bus_command.request_id },
						{ "error_message", (std::string)"not found" }
					});
				}
			}
		}
		if (arr.size() > 0) {
			json::Node result(arr);
			result.PrintNode(output);
		}
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