#pragma once
#include <string>
#include <sstream>
#include <vector>
#include <iostream>
#include "transport_catalogue.h"

namespace catalogue {
	namespace parse_structs {
		struct Stop;
		struct Bus;
		struct BusInfo;
		struct StopInfo;
	}
	class TransportCatalogue;
}

namespace commands {
	enum class WriteCommand;

	enum class ReadCommand;

	template <typename CommandType>
	struct Command;

	struct Commands;
}

namespace detail {
	std::string_view DeleteSpace(std::string_view input);

	std::vector<std::string_view> SplitBy(std::string_view input, std::string_view del);

	void ExecCommands(catalogue::TransportCatalogue& catalogue, const commands::Commands& commands);

	void LoadString(catalogue::TransportCatalogue& catalogue, std::istream& input);
}

namespace string_parser {
	namespace read {
		void stop(const commands::Command<commands::ReadCommand>& command, catalogue::TransportCatalogue& catalogue, std::ostream& stream);
		void bus(const commands::Command<commands::ReadCommand>& command, catalogue::TransportCatalogue& catalogue, std::ostream& stream);
	}
}
