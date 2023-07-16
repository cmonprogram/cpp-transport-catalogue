#include <string>
#include "transport_catalogue.h"
#include "request_handler.h"
#include "json_reader.h"

int main() {
    catalogue::TransportCatalogue catalogue;
    renderer::MapRenderer map;
    RequestHandler handler(catalogue, map, std::cout);
    JSONReader reader(std::cin);
    handler.ExecuteCommands(reader.LoadJson());
}