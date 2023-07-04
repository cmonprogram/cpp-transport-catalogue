#pragma once
#include <string>
#include <sstream>
#include <vector>
#include <iostream>

#include "transport_catalogue.h"
#include "request_handler.h"
#include "json.h"
#include "svg.h"

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

	svg::Color set_color(json::Node node);

	void LoadJson(RequestHandler& handler, std::istream& input, std::ostream& output = std::cout);
}

/*
 * Здесь можно разместить код наполнения транспортного справочника данными из JSON,
 * а также код обработки запросов к базе и формирование массива ответов в формате JSON
 */