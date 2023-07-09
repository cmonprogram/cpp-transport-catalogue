#include "map_renderer.h"

bool IsZero(double value) {
	return std::abs(value) < EPSILON;
}

void renderer::MapRenderer::SetSettings(const renderer::RenderSettings& settings) {
	settings_ = settings;
}

const renderer::RenderSettings& renderer::MapRenderer::GetSettings() {
	return settings_;
}


void renderer::MapRenderer::FormMap(const std::map<std::string_view, const catalogue::parse_structs::Bus*>& bus_list, std::ostream& out) const
{

	svg::Document doc;

	std::vector<svg::Polyline> lines;
	std::vector<svg::Text> bus_texts;
	std::map <std::string_view, svg::Circle> circles;
	std::map <std::string_view, std::pair<svg::Text, svg::Text>> stop_texts;

	auto& palette = settings_.color_palette;
	auto palette_value = palette.begin();
	/*
	std::sort(bus_list.begin(), bus_list.end(), [](const auto& lhs, const auto& rhs) {
		return lhs.name < rhs.name;
		});
	*/

	std::vector<geo::Coordinates> geo_coords;
	for (const auto& [bus_name, bus] : bus_list) {
		for (const auto& stop : bus->stops) {
			geo_coords.push_back(stop->coords);
		}
	}
	SphereProjector coords(geo_coords.begin(), geo_coords.end(), settings_.width, settings_.height, settings_.padding);
	for (const auto& [bus_name, bus] : bus_list) {
		if (bus->stops.size() == 0) continue;
		if (palette_value == palette.end()) palette_value = palette.begin();

		//Lines
		auto line = svg::Polyline();
		for (const auto& stop : bus->stops) {
			svg::Point screen_coord = coords(stop->coords);
			line.AddPoint(screen_coord);

			if (!circles.count(stop->name)) {
				//Circle
				auto circle = svg::Circle().SetCenter(screen_coord).SetRadius(settings_.stop_radius).SetFillColor("white");
				circles.insert({ stop->name, std::move(circle) });
			}
			if (!stop_texts.count(stop->name)) {
				//Stop Text
				auto [text1, text2] = PrepareStopText(coords(stop->coords));
				text1.SetData(stop->name);
				text2.SetData(stop->name);
				stop_texts.insert({ stop->name, {std::move(text1), std::move(text2)} });
			}
		}
		if (!bus->is_roundtrip) {
			for (auto iter = bus->stops.rbegin() + 1; iter != bus->stops.rend(); iter++) {
				const svg::Point screen_coord = coords((*iter)->coords);
				line.AddPoint(std::move(screen_coord));
			}
		}
		line.SetStrokeColor(*palette_value).SetFillColor("none").SetStrokeWidth(settings_.line_width).SetStrokeLineCap(svg::StrokeLineCap::ROUND).SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
		lines.push_back(std::move(line));
		//doc.Add(std::move(line));


		//Bus Text
		auto [text1, text2] = PrepareBusText(coords(bus->stops.at(0)->coords));
		text1.SetData(bus->name);
		text2.SetFillColor(*palette_value).SetData(bus->name);
		bus_texts.push_back(std::move(text1));
		bus_texts.push_back(std::move(text2));

		if (!bus->is_roundtrip && bus->stops.at(bus->stops.size() - 1) != bus->stops.at(0)) {
			auto [text3, text4] = PrepareBusText(coords(bus->stops.at(bus->stops.size() - 1)->coords));
			text3.SetData(bus->name);
			text4.SetFillColor(*palette_value).SetData(bus->name);
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
}


inline std::pair<svg::Text, svg::Text> renderer::MapRenderer::PrepareBusText(const svg::Point& point) const {
	auto text1 = svg::Text().SetFillColor(settings_.underlayer_color).SetStrokeColor(settings_.underlayer_color).SetStrokeWidth(settings_.underlayer_width).SetStrokeLineCap(svg::StrokeLineCap::ROUND).SetStrokeLineJoin(svg::StrokeLineJoin::ROUND).SetPosition(point).SetOffset(settings_.bus_label_offset).SetFontSize(settings_.bus_label_font_size).SetFontFamily("Verdana").SetFontWeight("bold");

	auto text2 = svg::Text().SetPosition(point).SetOffset(settings_.bus_label_offset).SetFontSize(settings_.bus_label_font_size).SetFontFamily("Verdana").SetFontWeight("bold");

	return { std::move(text1) , std::move(text2) };
}

inline std::pair<svg::Text, svg::Text> renderer::MapRenderer::PrepareStopText(const svg::Point& point) const {
	auto text1 = svg::Text().SetFillColor(settings_.underlayer_color).SetStrokeColor(settings_.underlayer_color).SetStrokeWidth(settings_.underlayer_width).SetStrokeLineCap(svg::StrokeLineCap::ROUND).SetStrokeLineJoin(svg::StrokeLineJoin::ROUND).SetPosition(point).SetOffset(settings_.stop_label_offset).SetFontSize(settings_.stop_label_font_size).SetFontFamily("Verdana");

	auto text2 = svg::Text().SetFillColor("black").SetPosition(point).SetOffset(settings_.stop_label_offset).SetFontSize(settings_.stop_label_font_size).SetFontFamily("Verdana");

	return { std::move(text1) , std::move(text2) };
}