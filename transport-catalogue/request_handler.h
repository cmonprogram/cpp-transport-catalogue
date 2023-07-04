#pragma once
#include "transport_catalogue.h"
#include "map_renderer.h"
#include "svg.h"

/*
 * ����� ����� ���� �� ���������� ��� ����������� �������� � ����, ����������� ������, ������� ��
 * �������� �� �������� �� � transport_catalogue, �� � json reader.
 *
 * � �������� ��������� ��� ���� ���������� ��������� �� ���� ������ ����������� ��������.
 * �� ������ ����������� ��������� �������� ��������, ������� ������� ���.
 *
 * ���� �� ������������� �������, ��� ����� ���� �� ��������� � ���� ����,
 * ������ �������� ��� ������.
 */

 // ����� RequestHandler ������ ���� ������, ����������� �������������� JSON reader-�
 // � ������� ������������ ����������.
 // ��. ������� �������������� �����: https://ru.wikipedia.org/wiki/�����_(������_��������������)



 class RequestHandler {
     struct SectionLoad {
         bool write_command = true;
         bool read_command = true;
         bool render_settings = true;
     };
 public:
     // MapRenderer ����������� � ��������� ����� ��������� �������
     RequestHandler(catalogue::TransportCatalogue& db, renderer::MapRenderer& renderer);
     // ���������� ���������� � �������� (������ Bus)
     std::optional<catalogue::parse_structs::BusInfo> GetBusStat(const std::string_view& bus_name) const;

     // ���������� ��������, ���������� �����
     const std::vector<const catalogue::parse_structs::Bus*> GetBusesByStop(const std::string_view& stop_name) const;

     // ���� ����� ����� ����� � ��������� ����� ��������� �������
     void FormMap(std::ostream& out) const;

      catalogue::TransportCatalogue&  GetCatalogue();
      renderer::MapRenderer& GetMap();
      SectionLoad is_loading;
 private:
     // RequestHandler ���������� ��������� �������� "������������ ����������" � "������������ �����"
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


