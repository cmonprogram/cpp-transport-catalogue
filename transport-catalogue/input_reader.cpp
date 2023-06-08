#include "input_reader.h"

void string_parser::write::stop(commands::Command<commands::WriteCommand> command, catalogue::TransportCatalogue& catalogue) {
	catalogue::parse_structs::Stop stop;
	std::string trash;
	std::istringstream s(command.text);
	std::getline(s, stop.name, ':');
	stop.name = detail::DeleteSpace(stop.name);
	s >> stop.coords.lat;
	s >> trash;
	s >> stop.coords.lng;

	catalogue.AddStop(stop);

	std::string distance;
	std::getline(s, distance);
	std::vector<std::string_view> distanses = detail::SplitBy(distance, ",");
	for (std::string_view val : distanses) {
		std::string string_val = std::string(val);
		std::istringstream s2(string_val);
		int value;
		std::string name;
		s2 >> value;
		s2 >> trash;
		s2 >> trash;
		std::getline(s2, name);
		name = detail::DeleteSpace(name);
		catalogue.SetDistance(stop.name, name, value);
	}
}

void string_parser::write::bus(commands::Command<commands::WriteCommand> command, catalogue::TransportCatalogue& catalogue) {
	catalogue::parse_structs::Bus bus;
	std::string trash;
	std::istringstream s(command.text);
	std::getline(s, bus.name, ':');
	bus.name = detail::DeleteSpace(bus.name);

	std::getline(s, trash);
	std::istringstream s2(trash);

	char delim = '>';
	//if (trash.find('>') != std::string::npos) {}
	if (trash.find('-') != std::string::npos) { delim = '-'; bus.is_reverse = true; }
	while (std::getline(s2, trash, delim)) {
		auto stop = &catalogue.GetStop(detail::DeleteSpace(trash));
		bus.stops.push_back(stop);
	}
	catalogue.AddBus(bus);
}
