#include "stat_reader.h"

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
	catalogue.AddBus(bus);
}


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

void detail::ExecCommands(catalogue::TransportCatalogue& catalogue, const commands::Commands& commands) {
	for (auto& command : commands.write) {
		if (command.type == commands::WriteCommand::Stop) {
			string_parser::write::stop(command, catalogue);
		}
	}

	for (auto& command : commands.write) {
		if (command.type == commands::WriteCommand::Bus) {
			string_parser::write::bus(command, catalogue);
		}
	}
	for (auto& command : commands.read) {
		if (command.type == commands::ReadCommand::Stop) {
			string_parser::read::stop(command, catalogue, std::cout);
		}
		if (command.type == commands::ReadCommand::Bus) {
			string_parser::read::bus(command, catalogue, std::cout);
		}

	}
}

void detail::LoadString(catalogue::TransportCatalogue& catalogue, std::istream& input) {
	commands::Commands commands;
	while (!input.eof()) {
		int command_size;
		input >> command_size;
		for (int i = 0; i < command_size; ++i) {
			std::string command_type;
			input >> command_type;

			std::string command_line;
			getline(input, command_line);

			commands::Command<commands::WriteCommand> command;
			command.text = std::move(command_line);

			if (command_type == "Bus") { command.type = commands::WriteCommand::Bus; }
			if (command_type == "Stop") { command.type = commands::WriteCommand::Stop; }
			commands.write.push_back(std::move(command));
		}

		input >> command_size;
		for (int i = 0; i < command_size; ++i) {
			std::string command_type;
			input >> command_type;

			std::string command_line;
			getline(input, command_line);

			commands::Command<commands::ReadCommand> command;
			command.text = std::move(command_line);

			if (command_type == "Bus") { command.type = commands::ReadCommand::Bus; }
			if (command_type == "Stop") { command.type = commands::ReadCommand::Stop; }
			commands.read.push_back(std::move(command));
		}
		detail::ExecCommands(catalogue, commands);
	}
}