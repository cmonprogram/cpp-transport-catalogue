#pragma once
#include "transport_catalogue.h"
#include "map_renderer.h"
#include "svg.h"

/*
 * Здесь можно было бы разместить код обработчика запросов к базе, содержащего логику, которую не
 * хотелось бы помещать ни в transport_catalogue, ни в json reader.
 *
 * В качестве источника для идей предлагаем взглянуть на нашу версию обработчика запросов.
 * Вы можете реализовать обработку запросов способом, который удобнее вам.
 *
 * Если вы затрудняетесь выбрать, что можно было бы поместить в этот файл,
 * можете оставить его пустым.
 */

 // Класс RequestHandler играет роль Фасада, упрощающего взаимодействие JSON reader-а
 // с другими подсистемами приложения.
 // См. паттерн проектирования Фасад: https://ru.wikipedia.org/wiki/Фасад_(шаблон_проектирования)



 class RequestHandler {
     struct SectionLoad {
         bool write_command = true;
         bool read_command = true;
         bool render_settings = true;
     };
 public:
     // MapRenderer понадобится в следующей части итогового проекта
     RequestHandler(catalogue::TransportCatalogue& db, renderer::MapRenderer& renderer);
     // Возвращает информацию о маршруте (запрос Bus)
     std::optional<catalogue::parse_structs::BusInfo> GetBusStat(const std::string_view& bus_name) const;

     // Возвращает маршруты, проходящие через
     const std::vector<const catalogue::parse_structs::Bus*> GetBusesByStop(const std::string_view& stop_name) const;

     // Этот метод будет нужен в следующей части итогового проекта
     void FormMap(std::ostream& out) const;

      catalogue::TransportCatalogue&  GetCatalogue();
      renderer::MapRenderer& GetMap();
      SectionLoad is_loading;
 private:
     // RequestHandler использует агрегацию объектов "Транспортный Справочник" и "Визуализатор Карты"
     catalogue::TransportCatalogue& db_;
     renderer::MapRenderer& renderer_;

     std::pair<svg::Text, svg::Text> prepare_bus_text(const svg::Point& point) const{
         auto settings = renderer_.GetSettings();
         auto text1 = svg::Text().SetFillColor(settings.underlayer_color).SetStrokeColor(settings.underlayer_color).SetStrokeWidth(settings.underlayer_width).SetStrokeLineCap(svg::StrokeLineCap::ROUND).SetStrokeLineJoin(svg::StrokeLineJoin::ROUND).SetPosition(point).SetOffset(settings.bus_label_offset).SetFontSize(settings.bus_label_font_size).SetFontFamily("Verdana").SetFontWeight("bold")/*.SetData("114")*/ ;

         auto text2 = svg::Text()/*.SetFillColor("green")*/.SetPosition(point).SetOffset(settings.bus_label_offset).SetFontSize(settings.bus_label_font_size).SetFontFamily("Verdana").SetFontWeight("bold")/*.SetData("114")*/ ;

         return { std::move(text1) , std::move(text2) };
     }

     std::pair<svg::Text, svg::Text> prepare_stop_text(const svg::Point& point) const {
         auto settings = renderer_.GetSettings();
         auto text1 = svg::Text().SetFillColor(settings.underlayer_color).SetStrokeColor(settings.underlayer_color).SetStrokeWidth(settings.underlayer_width).SetStrokeLineCap(svg::StrokeLineCap::ROUND).SetStrokeLineJoin(svg::StrokeLineJoin::ROUND).SetPosition(point).SetOffset(settings.stop_label_offset).SetFontSize(settings.stop_label_font_size).SetFontFamily("Verdana")/*.SetData("114")*/;

         auto text2 = svg::Text().SetFillColor("black").SetPosition(point).SetOffset(settings.stop_label_offset).SetFontSize(settings.stop_label_font_size).SetFontFamily("Verdana")/*.SetData("114")*/;

         return { std::move(text1) , std::move(text2) };
     }
 };


