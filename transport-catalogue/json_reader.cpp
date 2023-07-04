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

svg::Color detail::set_color(json::Node node) {

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



void detail::LoadJson(RequestHandler& handler, std::istream& input, std::ostream& output) {

	//необходимо для корректной работы стрима, без нарушения целостности
	//std::stringstream input;
	//input << input_stream.rdbuf();
	//std::string input_string = ss.str();

	auto document = json::Load(input);
	auto& root = document.GetRoot();
	auto& write_command = root.AsMap().at("base_requests").AsArray();
	auto& read_command = root.AsMap().at("stat_requests").AsArray();
	auto& render_settings = root.AsMap().at("render_settings").AsMap();
	auto& catalogue = handler.GetCatalogue();

	if (handler.is_loading.render_settings) {
		//for (auto& [key,val] : render_settings) {
			//auto& node = command.AsMap();
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
		settings.underlayer_color = detail::set_color(render_settings.at("underlayer_color"));
		settings.underlayer_width = render_settings.at("underlayer_width").AsDouble();
		auto& color_palette_arr = render_settings.at("color_palette").AsArray();
		for (auto& color : color_palette_arr) {
			settings.color_palette.emplace_back(detail::set_color(color));
		}

		auto& map = handler.GetMap();
		map.SetSettings(settings);
		
		//}
	}

	if (handler.is_loading.write_command) {
		for (auto& command : write_command) {
			auto& node = command.AsMap();
			if (node.at("type").AsString() == "Stop") {
				catalogue::parse_structs::Stop stop;
				stop.name = node.at("name").AsString();
				stop.coords = { node.at("latitude").AsDouble() , node.at("longitude").AsDouble() };

				catalogue.AddStop(stop);
				if (node.count("road_distances")) {
					for (auto& [name, val] : node.at("road_distances").AsMap()) {
						catalogue.SetDistance(stop.name, name, val.AsInt());
					}
				}
			}
		}

		for (auto& command : write_command) {
			auto& node = command.AsMap();
			if (node.at("type").AsString() == "Bus") {
				catalogue::parse_structs::Bus bus;
				bus.name = node.at("name").AsString();
				bus.is_reverse = true;
				bus.is_roundtrip = node.at("is_roundtrip").AsBool();

				auto& stops_array = node.at("stops").AsArray();
				int size = stops_array.size();

				if (bus.is_roundtrip) {
					bus.is_reverse = false;
				}
				for (int i = 0; i < size; ++i) {
					bus.stops.push_back(&catalogue.GetStop(stops_array.at(i).AsString()));
				}

				if (bus.stops.size() != 0) {

					for (size_t i = 1; i < bus.stops.size(); ++i) {
						bus.route_length += catalogue.GetDistance(bus.stops.at(i - 1)->name, bus.stops.at(i)->name);
						bus.direct_length += geo::ComputeDistance(bus.stops.at(i - 1)->coords, bus.stops.at(i)->coords);
					}

					if (bus.is_reverse) {
						for (size_t i = bus.stops.size() - 1; i > 0; --i) {
							bus.route_length += catalogue.GetDistance(bus.stops.at(i)->name, bus.stops.at(i - 1)->name);
							bus.direct_length += geo::ComputeDistance(bus.stops.at(i)->coords, bus.stops.at(i - 1)->coords);
						}
					}
				}
				catalogue.AddBus(bus);
			}
		}
	}
	if (handler.is_loading.read_command) {
		json::Array arr;
		for (auto& command : read_command) {
			auto& node = command.AsMap();


			if (node.at("type").AsString() == "Stop") {
				auto info = catalogue.GetStopInfo(node.at("name").AsString());
				if (info.is_found) {
					json::Array bus_names;
					for (const catalogue::parse_structs::Bus* bus : info.buses) {
						bus_names.emplace_back(bus->name);
					}
					arr.emplace_back(std::map<std::string, json::Node>{
						{"buses", bus_names},
						{ "request_id", node.at("id").AsInt() }
					});
				}
				else {
					if (info.is_exist) {
						arr.emplace_back(std::map<std::string, json::Node>{
							{ "request_id", node.at("id").AsInt() },
							{ "buses", json::Array{} }
						});
					}
					else {
						arr.emplace_back(std::map<std::string, json::Node>{
							{ "request_id", node.at("id").AsInt() },
							{ "error_message", (std::string)"not found" }
						});
					}

				}

			}

			if (node.at("type").AsString() == "Bus") {
				auto info = catalogue.GetBusInfo(node.at("name").AsString());
				if (info.is_found) {
					arr.emplace_back(std::map<std::string, json::Node>{
						{"curvature", info.route_length / info.direct_length},
						{ "request_id", node.at("id").AsInt() },
						{ "route_length", info.route_length },
						{ "stop_count", info.stops },
						{ "unique_stop_count", info.unique_stops }
					});
				}
				else {
					arr.emplace_back(std::map<std::string, json::Node>{
						{ "request_id", node.at("id").AsInt() },
						{ "error_message", (std::string)"not found" }
					});
				}


			}

			if (node.at("type").AsString() == "Map") {
				std::stringstream ss;
				handler.FormMap(ss);
				std::string map_string = ss.str();

				arr.emplace_back(std::map<std::string, json::Node>{
					{ "map", map_string},
					{ "request_id", node.at("id").AsInt() }
				});
			}
		}
		if (arr.size() > 0) {
			json::Node result(arr);
			result.PrintNode(output);
		}
	}





	/*
	* {
	  "width": 1200.0,
	  "height": 1200.0,

	  "padding": 50.0,

	  "line_width": 14.0,
	  "stop_radius": 5.0,

	  "bus_label_font_size": 20,
	  "bus_label_offset": [7.0, 15.0],

	  "stop_label_font_size": 20,
	  "stop_label_offset": [7.0, -3.0],

	  "underlayer_color": [255, 255, 255, 0.85],
	  "underlayer_width": 3.0,

	  "color_palette": [
		"green",
		[255, 160, 0],
		"red"
	  ]
	}
	*/


}
/*
 * Здесь можно разместить код наполнения транспортного справочника данными из JSON,
 * а также код обработки запросов к базе и формирование массива ответов в формате JSON
 */