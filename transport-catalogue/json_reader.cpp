#include "json_reader.h"
#include "request_handler.h"
#include <sstream>

std::string_view detail::DeleteSpace(std::string_view input) {
	int first = 0;
	int last = 0;
	//string_view rinput(input.rbegin(), input.rend())

	for (auto iter = input.begin(); iter != input.end(); ++iter) {
		if (*iter == ' ') ++first;
		else break;
	}

	for (auto iter = input.rbegin(); iter != input.rend(); ++iter) {
		if (*iter == ' ') ++last;
		else break;
	}

	return input.substr(first, input.size() - first - last);
}

std::vector<std::string_view> detail::SplitBy(std::string_view input, std::string_view del) {
	std::vector<std::string_view> result;
	size_t i = input.find(del);
	while (i != std::string_view::npos) {
		std::string_view part = input.substr(0, i);
		if (part != "") { result.push_back(DeleteSpace(part)); }
		input = input.substr(i + del.size(), input.size());
		i = input.find(del);
	}
	if (input.size()) { result.push_back(DeleteSpace(input)); }
	return result;
}

svg::Color detail::SetColor(json::Node node) {
	if (node.IsArray()) {
		auto& arr = node.AsArray();
		if (arr.size() == 3) {
			return svg::Color{svg::Rgb{arr.at(0).AsInt(), arr.at(1).AsInt(), arr.at(2).AsInt()}};
		}
		if (arr.size() == 4) {
			return svg::Color{svg::Rgba{arr.at(0).AsInt(), arr.at(1).AsInt(), arr.at(2).AsInt(), arr.at(3).AsDouble()}};
		}
		throw json::ParsingError("Wrong color");
	}
	return node.AsString();
}

JSONReader::JSONReader(std::istream& input) : input(input) {}

commands::Commands JSONReader::LoadJson() {

	commands::Commands commands;

	auto document = json::Load(input);
	auto& root = document.GetRoot();
	auto& write_command = root.AsDict().at("base_requests").AsArray();
	auto& read_command = root.AsDict().at("stat_requests").AsArray();
	auto& render_settings = root.AsDict().at("render_settings").AsDict();
	auto& routing_settings = root.AsDict().at("routing_settings").AsDict();
	if (commands.routing_settings.is_load) {
		commands.routing_settings.router_settings_comands.bus_wait_time = routing_settings.at("bus_wait_time").AsInt();
		commands.routing_settings.router_settings_comands.bus_velocity = routing_settings.at("bus_velocity").AsDouble();
	}
	if (commands.render_settings.is_load) {
		renderer::RenderSettings settings;

		settings.width = render_settings.at("width").AsDouble();
		settings.height = render_settings.at("height").AsDouble();
		settings.padding = render_settings.at("padding").AsDouble();
		settings.line_width = render_settings.at("line_width").AsDouble();
		settings.stop_radius = render_settings.at("stop_radius").AsDouble();
		settings.bus_label_font_size = render_settings.at("bus_label_font_size").AsDouble();
		settings.bus_label_offset = { render_settings.at("bus_label_offset").AsArray().at(0).AsDouble(), render_settings.at("bus_label_offset").AsArray().at(1).AsDouble() };
		settings.stop_label_font_size = render_settings.at("stop_label_font_size").AsDouble();
		settings.stop_label_offset = { render_settings.at("stop_label_offset").AsArray().at(0).AsDouble(), render_settings.at("stop_label_offset").AsArray().at(1).AsDouble() };
		settings.underlayer_color = detail::SetColor(render_settings.at("underlayer_color"));
		settings.underlayer_width = render_settings.at("underlayer_width").AsDouble();
		auto& color_palette_arr = render_settings.at("color_palette").AsArray();
		for (auto& color : color_palette_arr) {
			settings.color_palette.emplace_back(detail::SetColor(color));
		}

		commands.render_settings.render_settings_comands = std::move(settings);
	}

	if (commands.write_commands.is_load) {
		for (auto& command : write_command) {
			auto& node = command.AsDict();
			if (node.at("type").AsString() == "Stop") {
				catalogue::parse_structs::Stop stop;
				stop.name = node.at("name").AsString();
				stop.coords = { node.at("latitude").AsDouble() , node.at("longitude").AsDouble() };


				commands::WriteStopCommandInfo stop_and_distances;
				stop_and_distances.stop = std::move(stop);

				if (node.count("road_distances")) {
					for (auto& [name, val] : node.at("road_distances").AsDict()) {
						stop_and_distances.distances.push_back({ std::move(name), val.AsInt() });
					}
				}
				commands.write_commands.stops.push_back({ std::move(stop_and_distances) });
			}

			if (node.at("type").AsString() == "Bus") {
				commands::WriteBusCommandInfo bus;
				bus.name = node.at("name").AsString();
				bus.is_roundtrip = node.at("is_roundtrip").AsBool();
				auto& stops_array = node.at("stops").AsArray();
				int size = stops_array.size();
				for (int i = 0; i < size; ++i) {
					bus.stops.push_back(std::move(stops_array.at(i).AsString()));
				}
				commands.write_commands.buses.push_back(std::move(bus));
			}
		}
	}

	if (commands.read_commands.is_load) {
		for (auto& command : read_command) {
			auto& node = command.AsDict();
			if (node.at("type").AsString() == "Stop") {
				commands.read_commands.commands.push_back(commands::ReadStopCommandInfo{ node.at("id").AsInt(), std::move(node.at("name").AsString()) });
			}

			if (node.at("type").AsString() == "Bus") {
				commands.read_commands.commands.push_back(commands::ReadBusCommandInfo{ node.at("id").AsInt(), std::move(node.at("name").AsString()) });
			}

			if (node.at("type").AsString() == "Map") {
				commands.read_commands.commands.push_back(commands::ReadMapCommandInfo{ node.at("id").AsInt() });
			}

			if (node.at("type").AsString() == "Route") {
				commands.read_commands.commands.push_back(commands::ReadRouteCommandInfo{ 
					node.at("id").AsInt(), 
					node.at("from").AsString(),
					node.at("to").AsString()
				});
			}
		}

	}
	return commands;
}
