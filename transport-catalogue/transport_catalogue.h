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

#include "domain.h"
#include "geo.h"

const int hash_num = 37;
namespace catalogue {
	class TransportCatalogue {
	public:
		const parse_structs::Stop& GetStop(std::string_view name) const;
		double GetDistance(const std::string& stop_name_1, const std::string& stop_name_2) const;
		void SetDistance(const std::string& stop_name_1, const std::string& stop_name_2, int value);
		const parse_structs::Bus& GetBus(std::string_view name) const;
		parse_structs::StopInfo GetStopInfo(std::string_view name) const;
		parse_structs::BusInfo GetBusInfo(std::string_view name) const;
		const std::deque<parse_structs::Bus>& GetBusList() const;
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