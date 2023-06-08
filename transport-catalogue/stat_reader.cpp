#include "stat_reader.h"

void string_parser::read::stop(commands::Command<commands::ReadCommand> command, catalogue::TransportCatalogue& catalogue) {
	catalogue::parse_structs::StopInfo info = catalogue.GetStopInfo(detail::DeleteSpace(command.text));
	std::cout << "Stop " << info.name << ": ";
	if (info.is_found) {
		std::cout << "buses";
		for (const catalogue::parse_structs::Bus* bus : info.buses) {
			std::cout << " " << bus->name;
		}
		std::cout << std::endl;
	}
	else {
		if (info.is_exist) {
			std::cout << "no buses" << std::endl;
		}
		else {
			std::cout << "not found" << std::endl;
		}
	}
}

void string_parser::read::bus(commands::Command<commands::ReadCommand> command, catalogue::TransportCatalogue& catalogue) {
	catalogue::parse_structs::BusInfo info = catalogue.GetBusInfo(detail::DeleteSpace(command.text));
	if (info.is_found) {
		std::cout << "Bus " << info.name << ": " << info.stops << " stops on route, " << info.unique_stops << " unique stops, " << info.route_length << " route length, " << info.route_length / info.direct_length << " curvature" << std::endl;
	}
	else {
		std::cout << "Bus " << info.name << ": " << "not found" << std::endl;
	}
}
