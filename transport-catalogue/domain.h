#pragma once
#include <string>
#include <vector>
#include "geo.h"
#include "svg.h"

namespace catalogue {
	namespace parse_structs {
		struct Stop {
			Stop() = default;
			Stop(const std::string& name, double latitude, double longitude) : name(name), coords(latitude, longitude) {}
			std::string name;
			geo::Coordinates coords;
			//std::unordered_map<Stop*, int> distance;
		};

		struct Bus {
			Bus() = default;
			Bus(const std::string& name, const std::vector<const Stop*>& stops) : name(name), stops(stops) {}
			bool is_reverse = false;
			bool is_roundtrip = false;
			std::string name;
			std::vector<const Stop*> stops;
			int stops_count = 0;
			double route_length = 0;
			double direct_length = 0;
		};

		struct BusInfo {
			bool is_found = true;
			std::string_view name;
			int stops = 0;
			int unique_stops = 0;
			double route_length = 0;
			double direct_length = 0;
		};
		struct StopInfo {
			bool is_found = true;
			bool is_exist = true;
			std::string_view name;
			std::vector<const Bus*> buses;
		};
		struct StopDistance {
			std::string destination_stop;
			int value;
		};
	}
}

namespace renderer {
	struct RenderSettings {
		double width = 0;
		double height = 0;
		double padding = 0;

		double line_width = 0;
		double stop_radius = 0;

		double bus_label_font_size = 0;
		svg::Point bus_label_offset;

		double stop_label_font_size = 0;
		svg::Point  stop_label_offset;

		svg::Color underlayer_color;
		double underlayer_width = 0;

		std::vector<svg::Color> color_palette;
	};
}

namespace commands {
	struct WriteStopCommandInfo {
		catalogue::parse_structs::Stop stop;
		std::vector<catalogue::parse_structs::StopDistance> distances;
	};
	struct WriteBusCommandInfo {
		bool is_roundtrip = false;
		std::string name;
		std::vector<std::string> stops;
	};


	struct ReadCommand {
		int request_id;
	};

	struct ReadMapCommandInfo : public ReadCommand {
	};

	struct ReadStopCommandInfo : public ReadCommand {
		std::string stop_name;
	};

	struct ReadBusCommandInfo : public ReadCommand {
		std::string bus_name;
	};

	


	struct Commands {
		struct RenderCommands {
			renderer::RenderSettings render_settings_comands;
			bool render_settings_load = true;
		};
		struct WriteCommands {
			std::vector<WriteStopCommandInfo> stops;
			std::vector<WriteBusCommandInfo> buses;
			bool write_command_load = true;
		};



		struct ReadCommands {
			std::vector<std::variant<ReadMapCommandInfo, ReadStopCommandInfo, ReadBusCommandInfo >> commands;
			bool read_command_load = true;
		};

		RenderCommands render_settings;
		WriteCommands write_commands;
		ReadCommands read_commands;
	};
}