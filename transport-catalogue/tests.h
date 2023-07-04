#include <cassert>
#include "transport_catalogue.h"

void TransportCatalogue_AddStop() {
	catalogue::TransportCatalogue catalogue;
	catalogue.AddStop({ "Mockow", 55, 37 });
	catalogue.AddStop({ "Spb", 59, 30 });
	catalogue.AddStop({ "Yar", 57, 39 });

	assert([&]() {
		catalogue::parse_structs::Stop stop = catalogue.GetStop("Mockow");
		return stop.name == "Mockow" && stop.coords.lat == 55 && stop.coords.lng == 37;
		}());
}