#pragma once
#include <string>
#include <string_view>
#include <vector>
#include <deque>
#include <unordered_map>
#include <unordered_set>
#include <set>
#include <algorithm>
#include <sstream>

#include "geo.h"
#include "input_reader.h"
#include "stat_reader.h"

const int hash_num = 37;

namespace catalogue {
	class TransportCatalogue;
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




namespace detail {
	std::string_view DeleteSpace(std::string_view input);

	std::vector<std::string_view> SplitBy(std::string_view input, std::string_view del);

	void ExecCommands(catalogue::TransportCatalogue& catalogue, const commands::Commands& commands);

	void LoadString(catalogue::TransportCatalogue& catalogue, std::istream& input);
}

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
			std::string name;
			std::vector<const Stop*> stops;
		};

		struct BusInfo {
			bool is_found = true;
			std::string_view name;
			size_t stops = 0;
			size_t unique_stops = 0;
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


	class TransportCatalogue {
		friend void detail::ExecCommands(catalogue::TransportCatalogue& catalogue, const commands::Commands& commands);
	public:
		const parse_structs::Stop& GetStop(std::string_view name) const;
		double GetDistance(const std::string& stop_name_1, const std::string& stop_name_2) const;
		void SetDistance(const std::string& stop_name_1, const std::string& stop_name_2, int value);
		const parse_structs::Bus& GetBus(std::string_view name) const;
		parse_structs::StopInfo GetStopInfo(std::string_view name) const;
		parse_structs::BusInfo GetBusInfo(std::string_view name) const;

		void AddStop(parse_structs::Stop stop);
		void AddBus(parse_structs::Bus bus);

	private:
		std::deque<parse_structs::Stop> stop_base;
		std::unordered_map<std::string_view, const parse_structs::Stop*> stop_name_view;

		std::deque<parse_structs::Bus> bus_base;
		std::unordered_map<std::string_view, const parse_structs::Bus*> bus_name_view;
		std::unordered_map<std::string_view, std::unordered_set<const parse_structs::Bus*>> stop_bus_view;

		struct distance_view_hasher {
			size_t operator()(const std::pair<std::string, std::string> val) const {
				return std::hash<std::string>()(val.first) + std::hash<std::string>()(val.second) * hash_num;
			}
		};
		std::unordered_map<std::pair<std::string, std::string>, int, distance_view_hasher> distance_view;
	};
}

