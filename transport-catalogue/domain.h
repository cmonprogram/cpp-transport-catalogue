#pragma once
#include <string>
#include <vector>
#include "geo.h"


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

	}
}

namespace commands {
	enum class WriteCommand {
		Bus,
		Stop
	};

	enum class ReadCommand {
		Bus,
		Stop
	};

	template <typename CommandType>
	struct Command {
		std::string text;
		CommandType type;
	};

	struct Commands {
		std::vector<Command<WriteCommand>> write;
		std::vector<Command<ReadCommand>> read;
	};

}