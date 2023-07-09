#pragma once
#include "transport_catalogue.h"
#include "map_renderer.h"
#include "svg.h"
#include "json.h"
class RequestHandler {

public:
	RequestHandler(catalogue::TransportCatalogue& db, renderer::MapRenderer& renderer, std::ostream& output = std::cout);
	void ExecuteCommands(commands::Commands commands);

	std::optional<catalogue::parse_structs::BusInfo> GetBusStat(const std::string_view& bus_name) const;
	const std::vector<const catalogue::parse_structs::Bus*> GetBusesByStop(const std::string_view& stop_name) const;

private:
	// RequestHandler использует агрегацию объектов "Транспортный Справочник" и "Визуализатор Карты"
	catalogue::TransportCatalogue& db_;
	renderer::MapRenderer& renderer_;
	//std::istream& input = std::cin;
	std::ostream& output = std::cout;
};