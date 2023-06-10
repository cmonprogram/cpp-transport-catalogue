#include <ostream>
#include "stat_reader.h"

void string_parser::read::stop(const commands::Command<commands::ReadCommand>& command, catalogue::TransportCatalogue& catalogue, std::ostream& stream) {
	catalogue::parse_structs::StopInfo info = catalogue.GetStopInfo(detail::DeleteSpace(command.text));
	stream << "Stop " << info.name << ": ";
	if (info.is_found) {
		stream << "buses";
		for (const catalogue::parse_structs::Bus* bus : info.buses) {
			stream << " " << bus->name;
		}
		stream << std::endl;
	}
	else {
		if (info.is_exist) {
			stream << "no buses" << std::endl;
		}
		else {
			stream << "not found" << std::endl;
		}
	}
}

void string_parser::read::bus(const commands::Command<commands::ReadCommand>& command, catalogue::TransportCatalogue& catalogue, std::ostream& stream) {
	catalogue::parse_structs::BusInfo info = catalogue.GetBusInfo(detail::DeleteSpace(command.text));
	if (info.is_found) {
		stream << "Bus " << info.name << ": " << info.stops << " stops on route, " << info.unique_stops << " unique stops, " << info.route_length << " route length, " << info.route_length / info.direct_length << " curvature" << std::endl;
	}
	else {
		stream << "Bus " << info.name << ": " << "not found" << std::endl;
	}
}
