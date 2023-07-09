#include "transport_catalogue.h"


namespace catalogue {
	const parse_structs::Stop& catalogue::TransportCatalogue::GetStop(std::string_view name) const {
		if (!stop_name_view.count(name)) throw std::out_of_range("Not Found");
		return *(stop_name_view.at(name));
	}
	double TransportCatalogue::GetDistance(const std::string& stop_name_1, const std::string& stop_name_2) const {
		auto item = distance_view.find({ stop_name_1, stop_name_2 });
		if (item == distance_view.end()) {
			item = distance_view.find({ stop_name_2,  stop_name_1 });
		}
		if (item != distance_view.end()) {
			return (*item).second;
		}
		else {
			return geo::ComputeDistance(stop_name_view.at(stop_name_1)->coords, stop_name_view.at(stop_name_2)->coords);
		}
	}
	void TransportCatalogue::SetDistance(const std::string& stop_name_1, const std::string& stop_name_2, int value) {
		distance_view.insert({ { stop_name_1, stop_name_2 }, value });
	}
	const parse_structs::Bus& TransportCatalogue::GetBus(std::string_view name) const {
		if (!bus_name_view.count(name)) throw std::out_of_range("Not Found");
		return *(bus_name_view.at(name));
	}
	parse_structs::StopInfo TransportCatalogue::GetStopInfo(std::string_view name) const {
		parse_structs::StopInfo stop_info;
		stop_info.name = name;
		if (stop_bus_view.count(name)) {
			std::unordered_set<const parse_structs::Bus*> bus_set = stop_bus_view.at(name);
			stop_info.buses = std::vector<const parse_structs::Bus*>(bus_set.begin(), bus_set.end());
			std::sort(stop_info.buses.begin(), stop_info.buses.end(), [](const auto& lhs, const auto& rhs) {
				return	lhs->name < rhs->name;
				});
		}
		else {
			stop_info.is_found = false;
			if (!stop_name_view.count(name)) {
				stop_info.is_exist = false;
			}
		}
		return stop_info;
	}
	parse_structs::BusInfo TransportCatalogue::GetBusInfo(std::string_view name) const {
		try {
			const parse_structs::Bus& bus = GetBus(name);
			parse_structs::BusInfo bus_info;
			bus_info.name = bus.name;
			bus_info.route_length = bus.route_length;
			bus_info.direct_length = bus.direct_length;

			bus_info.stops = bus.stops.size();
			if (bus.is_reverse) {
				bus_info.stops += bus.stops.size() - 1;
			}
			bus_info.unique_stops = std::set(bus.stops.begin(), bus.stops.end()).size();
			return bus_info;
		}
		catch (std::out_of_range& e) {
			parse_structs::BusInfo bus_info;
			bus_info.is_found = false;
			bus_info.name = name;
			return bus_info;
		}
	}
	const std::map<std::string_view, const parse_structs::Bus*>& TransportCatalogue::GetBusList() const
	{
		return bus_name_view;
	}
	void TransportCatalogue::AddStop(parse_structs::Stop stop) {
		//void AddStop(const std::string& name, double latitude, double longitude) {
		stop_base.push_back(std::move(stop));
		auto& stop_added = stop_base.back();
		stop_name_view[stop_added.name] = &stop_added;
		//return &stop_added;
	}
	void TransportCatalogue::AddBus(parse_structs::Bus bus) {
		//void AddBus(const std::string& name, const std::vector<std::string>& stop_names) {
		bus_base.push_back(std::move(bus));
		auto& bus_added = bus_base.back();
		bus_name_view[bus_added.name] = &bus_added;

		for (const parse_structs::Stop* stop : bus_added.stops) {
			stop_bus_view[stop->name].insert(&bus_added);
		}
		//return &bus_added;
	}
}
