#pragma once
#include "geo.h"
#include "svg.h"

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <optional>
#include <vector>

inline const double EPSILON = 1e-6;
bool IsZero(double value);

class SphereProjector {
public:
    // points_begin � points_end ������ ������ � ����� ��������� ��������� geo::Coordinates
    template <typename PointInputIt>
    SphereProjector(PointInputIt points_begin, PointInputIt points_end,
        double max_width, double max_height, double padding)
        : padding_(padding) //
    {
        // ���� ����� ����������� ����� �� ������, ��������� ������
        if (points_begin == points_end) {
            return;
        }

        // ������� ����� � ����������� � ������������ ��������
        const auto [left_it, right_it] = std::minmax_element(
            points_begin, points_end,
            [](auto lhs, auto rhs) { return lhs.lng < rhs.lng; });
        min_lon_ = left_it->lng;
        const double max_lon = right_it->lng;

        // ������� ����� � ����������� � ������������ �������
        const auto [bottom_it, top_it] = std::minmax_element(
            points_begin, points_end,
            [](auto lhs, auto rhs) { return lhs.lat < rhs.lat; });
        const double min_lat = bottom_it->lat;
        max_lat_ = top_it->lat;

        // ��������� ����������� ��������������� ����� ���������� x
        std::optional<double> width_zoom;
        if (!IsZero(max_lon - min_lon_)) {
            width_zoom = (max_width - 2 * padding) / (max_lon - min_lon_);
        }

        // ��������� ����������� ��������������� ����� ���������� y
        std::optional<double> height_zoom;
        if (!IsZero(max_lat_ - min_lat)) {
            height_zoom = (max_height - 2 * padding) / (max_lat_ - min_lat);
        }

        if (width_zoom && height_zoom) {
            // ������������ ��������������� �� ������ � ������ ���������,
            // ���� ����������� �� ���
            zoom_coeff_ = std::min(*width_zoom, *height_zoom);
        }
        else if (width_zoom) {
            // ����������� ��������������� �� ������ ���������, ���������� ���
            zoom_coeff_ = *width_zoom;
        }
        else if (height_zoom) {
            // ����������� ��������������� �� ������ ���������, ���������� ���
            zoom_coeff_ = *height_zoom;
        }
    }

    // ���������� ������ � ������� � ���������� ������ SVG-�����������
    svg::Point operator()(geo::Coordinates coords) const {
        return {
            (coords.lng - min_lon_) * zoom_coeff_ + padding_,
            (max_lat_ - coords.lat) * zoom_coeff_ + padding_
        };
    }

private:
    double padding_;
    double min_lon_ = 0;
    double max_lat_ = 0;
    double zoom_coeff_ = 0;
};




namespace renderer {
    struct RenderSettings {
        double width = 0;
        double height = 0;
        double padding = 0;

        double line_width = 0;
        double stop_radius = 0;

        double bus_label_font_size = 0;
        svg::Point bus_label_offset;

        double stop_label_font_size = 0;
        svg::Point  stop_label_offset;

        svg::Color underlayer_color;
        double underlayer_width = 0;

        std::vector<svg::Color> color_palette;

        /*
        {
          "width": 1200.0,
          "height": 1200.0,

          "padding": 50.0,

          "line_width": 14.0,
          "stop_radius": 5.0,

          "bus_label_font_size": 20,
          "bus_label_offset": [7.0, 15.0],

          "stop_label_font_size": 20,
          "stop_label_offset": [7.0, -3.0],

          "underlayer_color": [255, 255, 255, 0.85],
          "underlayer_width": 3.0,

          "color_palette": [
            "green",
            [255, 160, 0],
            "red"
          ]
        }
        */ 


    };
    class MapRenderer {
    public:
        MapRenderer() = default;
        void SetSettings(RenderSettings settings) {
            settings_ = settings;
        }
        RenderSettings& GetSettings() {
            return settings_;
        }
        /*
        template <typename PointInputIt>
        void Render(PointInputIt points_begin, PointInputIt points_end) {
            SphereProjector coords(points_begin, points_end, renderer_.settings_.width, settings_.height, settings_.padding);
        }
        */
        /*
        void SetInterval(const std::vector<geo::Coordinates>& geo_coords) {
            coords_ = SphereProjector(geo_coords.begin(), geo_coords.end(), settings_.width, settings_.height, settings_.padding);
        }
        */
    private:
        RenderSettings settings_;
        //��������� � ���� ������, ��� ���������������� SphereProjector ����� ������ ���� �����������, ������� ������ ���� �� � ���� �������
       // SphereProjector coords_;
    };
}