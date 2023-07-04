#include "request_handler.h"
#include <map>

/*
 * Здесь можно было бы разместить код обработчика запросов к базе, содержащего логику, которую не
 * хотелось бы помещать ни в transport_catalogue, ни в json reader.
 *
 * Если вы затрудняетесь выбрать, что можно было бы поместить в этот файл,
 * можете оставить его пустым.
 */

void RequestHandler::FormMap(std::ostream& out) const
{

	svg::Document doc;

	std::vector<svg::Polyline> lines;
	std::vector<svg::Text> bus_texts;
	std::map <std::string_view, svg::Circle> circles;
	std::map <std::string_view, std::pair<svg::Text, svg::Text>> stop_texts;

	auto settings = renderer_.GetSettings();
	auto bus_list = db_.GetBusList();
	auto& palette = settings.color_palette;
	auto palette_value = palette.begin();

	std::sort(bus_list.begin(), bus_list.end(), [](const auto& lhs, const auto& rhs) {
		return lhs.name < rhs.name;
		});

	std::vector<geo::Coordinates> geo_coords;
	for (const auto& bus : bus_list) {
		for (const auto& stop : bus.stops) {
			geo_coords.push_back(stop->coords);
		}
	}
	SphereProjector coords(geo_coords.begin(), geo_coords.end(), settings.width, settings.height, settings.padding);
	for (const auto& bus : bus_list) {
		if (bus.stops.size() == 0) continue;
		if (palette_value == palette.end()) palette_value = palette.begin();

		//Lines
		auto line = svg::Polyline();
		for (const auto& stop : bus.stops) {
			const svg::Point screen_coord = coords(stop->coords);
			line.AddPoint(std::move(screen_coord));

			if (!circles.count(stop->name)) {
				//Circle
				auto circle = svg::Circle().SetCenter(screen_coord).SetRadius(settings.stop_radius).SetFillColor("white");
				circles.insert({ stop->name, std::move(circle) });
			}
			if (!stop_texts.count(stop->name)) {
				//Stop Text
				auto [text1, text2] = prepare_stop_text(coords(stop->coords));
				text1.SetData(stop->name);
				text2.SetData(stop->name);
				stop_texts.insert({ stop->name, {std::move(text1), std::move(text2)}  });
			}
		}
		if (!bus.is_roundtrip) {
			for (auto iter = bus.stops.rbegin() + 1; iter != bus.stops.rend(); iter++) {
				const svg::Point screen_coord = coords((*iter)->coords);
				line.AddPoint(std::move(screen_coord));
			}
		}
		line.SetStrokeColor(*palette_value).SetFillColor("none").SetStrokeWidth(settings.line_width).SetStrokeLineCap(svg::StrokeLineCap::ROUND).SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
		lines.push_back(std::move(line));
		//doc.Add(std::move(line));


		//Bus Text
		auto [text1, text2] = prepare_bus_text(coords(bus.stops.at(0)->coords));
		text1.SetData(bus.name);
		text2.SetFillColor(*palette_value).SetData(bus.name);
		bus_texts.push_back(std::move(text1));
		bus_texts.push_back(std::move(text2));

		if (!bus.is_roundtrip && bus.stops.at(bus.stops.size() - 1) != bus.stops.at(0)) {
			auto [text3, text4] = prepare_bus_text(coords(bus.stops.at(bus.stops.size() - 1)->coords));
			text3.SetData(bus.name);
			text4.SetFillColor(*palette_value).SetData(bus.name);
			bus_texts.push_back(std::move(text3));
			bus_texts.push_back(std::move(text4));
		}
		++palette_value;
	}

	for (auto& line : lines) {
		doc.Add(std::move(line));
	}
	for (auto& text : bus_texts) {
		doc.Add(std::move(text));
	}

	for (auto& circle : circles) {
		doc.Add(std::move(circle.second));
	}
	for (auto& text : stop_texts) {
		doc.Add(std::move(text.second.first));
		doc.Add(std::move(text.second.second));
	}

	doc.Render(out);
	//return std::move(doc);
	//return doc;
	//return svg::Document();
}

catalogue::TransportCatalogue& RequestHandler::GetCatalogue() {
	return db_;
}

renderer::MapRenderer& RequestHandler::GetMap()
{
	return renderer_;
}

RequestHandler::RequestHandler(catalogue::TransportCatalogue& db, renderer::MapRenderer& renderer) : db_(db), renderer_(renderer)
{
}

std::optional<catalogue::parse_structs::BusInfo> RequestHandler::GetBusStat(const std::string_view& bus_name) const
{
	return db_.GetBusInfo(bus_name);
}

const std::vector<const catalogue::parse_structs::Bus*> RequestHandler::GetBusesByStop(const std::string_view& stop_name) const
{
	return db_.GetStopInfo(stop_name).buses;
}
