#include <string>
#include "transport_catalogue.h"
#include "request_handler.h"
#include "json_reader.h"

int main() {
    /*
     * Примерная структура программы:
     *
     * Считать JSON из stdin
     * Построить на его основе JSON базу данных транспортного справочника
     * Выполнить запросы к справочнику, находящиеся в массиве "stat_requests", построив JSON-массив
     * с ответами.
     * Вывести в stdout ответы в виде JSON
     */
    const std::string input = R"V0G0N({
    "base_requests": [
      {
        "type": "Bus",
        "name": "114",
        "stops": ["Морской вокзал", "Ривьерский мост"],
        "is_roundtrip": false
      },
      {
        "type": "Stop",
        "name": "Ривьерский мост",
        "latitude": 43.587795,
        "longitude": 39.716901,
        "road_distances": {"Морской вокзал": 850}
      },
      {
        "type": "Stop",
        "name": "Морской вокзал",
        "latitude": 43.581969,
        "longitude": 39.719848,
        "road_distances": {"Ривьерский мост": 850}
      }
    ],
    "render_settings": {
      "width": 200,
      "height": 200,
      "padding": 30,
      "stop_radius": 5,
      "line_width": 14,
      "bus_label_font_size": 20,
      "bus_label_offset": [7, 15],
      "stop_label_font_size": 20,
      "stop_label_offset": [7, -3],
      "underlayer_color": [255,255,255,0.85],
      "underlayer_width": 3,
      "color_palette": ["green", [255,160,0],"red"]
    },
    "stat_requests": [
      { "id": 1, "type": "Map" },
      { "id": 2, "type": "Stop", "name": "Ривьерский мост" },
      { "id": 3, "type": "Bus", "name": "114" }
    ]
  })V0G0N";

    const std::string input2 = R"V0G0N({
    {"base_requests": [{
"type": "Bus", "name": "ABC", "stops": [], "is_roundtrip": true
}],  "stat_requests": []}
)V0G0N";


    /*
    13
    Stop Tolstopaltsevo: 55.611087, 37.20829, 3900m to Marushkino
    Stop Marushkino: 55.595884, 37.209755, 9900m to Rasskazovka, 100m to Marushkino
    Bus 256: Biryulyovo Zapadnoye > Biryusinka > Universam > Biryulyovo Tovarnaya > Biryulyovo Passazhirskaya > Biryulyovo Zapadnoye
    Bus 750: Tolstopaltsevo - Marushkino - Marushkino - Rasskazovka
    Stop Rasskazovka: 55.632761, 37.333324, 9500m to Marushkino
    Stop Biryulyovo Zapadnoye: 55.574371, 37.6517, 7500m to Rossoshanskaya ulitsa, 1800m to Biryusinka, 2400m to Universam
    Stop Biryusinka: 55.581065, 37.64839, 750m to Universam
    Stop Universam: 55.587655, 37.645687, 5600m to Rossoshanskaya ulitsa, 900m to Biryulyovo Tovarnaya
    Stop Biryulyovo Tovarnaya: 55.592028, 37.653656, 1300m to Biryulyovo Passazhirskaya
    Stop Biryulyovo Passazhirskaya: 55.580999, 37.659164, 1200m to Biryulyovo Zapadnoye
    Bus 828: Biryulyovo Zapadnoye > Universam > Rossoshanskaya ulitsa > Biryulyovo Zapadnoye
    Stop Rossoshanskaya ulitsa: 55.595579, 37.605757
    Stop Prazhskaya: 55.611678, 37.603831
    6
    Bus 256
    Bus 750
    Bus 751
    Stop Samara
    Stop Prazhskaya
    Stop Biryulyovo Zapadnoye
    */

    /*
    std::istringstream strm(input);
    auto document =  json::Load(strm);
    auto root = document.GetRoot();
    root.AsMap().at("base_requests").AsArray().at(0).AsMap().at("type").PrintNode(std::cout);
    */
    /*
    catalogue::TransportCatalogue catalogue;
    std::istringstream strm(input);
    detail::LoadJson(catalogue, strm);
    auto info = catalogue.GetBusInfo("114");

    json::Array arr;

    arr.emplace_back(std::map<std::string, json::Node>{
        {"buses", {}},
        {"request_id", 0}
    });


    arr.emplace_back(std::map<std::string, json::Node>{
            {"curvature", info.route_length / info.direct_length},
            { "request_id", 0 },
            { "route_length", info.route_length},
            { "stop_count", info.stops },
            { "unique_stop_count", info.unique_stops }
        });

    json::Node node(arr);
    node.PrintNode(std::cout);
    */

    catalogue::TransportCatalogue catalogue;
    renderer::MapRenderer map;
    RequestHandler handler(catalogue, map);

    std::istringstream strm(input);
    //detail::LoadJson(handler, strm);
    detail::LoadJson(handler, std::cin);

}