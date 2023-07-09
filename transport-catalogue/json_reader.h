#pragma once
#include <string>
#include <sstream>
#include <vector>
#include <iostream>

#include "request_handler.h"
#include "json.h"
#include "svg.h"
#include "domain.h"


namespace detail {
	std::string_view DeleteSpace(std::string_view input);

	std::vector<std::string_view> SplitBy(std::string_view input, std::string_view del);

	svg::Color SetColor(json::Node node);
}

class JSONReader {
private:
	std::istream& input = std::cin;

public:
	JSONReader(std::istream& input = std::cin);
	commands::Commands LoadJson();
};