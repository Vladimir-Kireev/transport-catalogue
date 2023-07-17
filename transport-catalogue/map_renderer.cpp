#include "map_renderer.h"

namespace renderer {

using namespace std::literals;

void MapRenderer::RenderCoords(const std::deque<Bus>& routes) {
	routes_.reserve(routes.size());
    for(const auto& bus : routes) {
        routes_.push_back(&bus);
    }
    std::sort(routes_.begin(), routes_.end(), [](const Bus* lhs, const Bus* rhs) {
		return lhs->number < rhs->number; });
    
	std::vector<const Stop*> geo_coords;
	std::vector<geo::Coordinates> coord_to_sphere_pro;
	
	for (const auto& rout : routes_) {
		for (const auto& stop : rout->stops) {
			stops_.insert(stop->name);
			geo_coords.push_back(stop);
			coord_to_sphere_pro.push_back(stop->coordinates);
		}
	}
	proj_ = { coord_to_sphere_pro.begin(), coord_to_sphere_pro.end(), settings_.width, settings_.height, settings_.padding };
	
	SetSphereProjector(geo_coords);
}

void MapRenderer::AddSetting(MapSetting settings) {
	settings_ = std::move(settings);
}

void MapRenderer::SetSphereProjector(const std::vector<const Stop*>& geo_coords) {
	for (const auto& stop : geo_coords) {
		sphere_points_.push_back(std::move(proj_(stop->coordinates)));
		stop_to_point_[stop->name] = &sphere_points_.back();
	}
}

void MapRenderer::SetPolyline() {
	for (size_t i = 0; i < routes_.size(); ++i) {
		size_t i_color = i;
		if (i >= settings_.color_palette.size()) {
			i_color -= settings_.color_palette.size() * (i / settings_.color_palette.size());
		}

		svg::Polyline polyline;
		for (const auto& stop : routes_[i]->stops) {
			polyline.AddPoint(*stop_to_point_.at(stop->name));
		}
		if (!routes_[i]->is_ring) {
			for (auto it = ++routes_[i]->stops.rbegin(); it != routes_[i]->stops.rend(); ++it) {  // добавляем обратную дорогу
				polyline.AddPoint(*stop_to_point_.at((*it)->name));
			}
		}
		polyline.SetStrokeColor(settings_.color_palette[i_color])
			.SetStrokeWidth(settings_.line_width)
			.SetFillColor(svg::NoneColor)
			.SetStrokeLineCap(svg::StrokeLineCap::ROUND)
			.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
		map_objects_.Add(polyline);
	}
}

void MapRenderer::AddRoutText(const std::string_view& stop_name, const std::string& rout, int color_num) {
	svg::Text text;
	svg::Text back_text;
	text.SetPosition(*stop_to_point_.at(stop_name))
		.SetOffset(settings_.bus_label_offset)
		.SetFontSize(settings_.bus_label_font_size)
		.SetFontFamily("Verdana"s)
		.SetFontWeight("bold"s)
		.SetData(rout)
		.SetFillColor(settings_.color_palette[color_num]);
	
	back_text.SetPosition(*stop_to_point_.at(stop_name))
		.SetOffset(settings_.bus_label_offset)
		.SetFontSize(settings_.bus_label_font_size)
		.SetFontFamily("Verdana"s)
		.SetFontWeight("bold"s)
		.SetData(rout)
		.SetFillColor(settings_.underlayer_color)
		.SetStrokeColor(settings_.underlayer_color)
		.SetStrokeWidth(settings_.underlayer_width)
		.SetStrokeLineCap(svg::StrokeLineCap::ROUND)
		.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

	map_objects_.Add(back_text);
	map_objects_.Add(text);
}

void MapRenderer::SetRoutes() {
	for (size_t i = 0; i < routes_.size(); ++i) {
		size_t i_color = i >= settings_.color_palette.size() ?
			i - settings_.color_palette.size() * (i / settings_.color_palette.size())
			: i;
		if (routes_[i]->is_ring || routes_[i]->stops.front()->name == routes_[i]->stops.back()->name) {
			std::string_view stop_name = routes_[i]->stops.front()->name;
			AddRoutText(stop_name, routes_[i]->number, i_color);
		}
		else {
			std::string_view stop_name1 = routes_[i]->stops.front()->name;
			std::string_view stop_name2 = routes_[i]->stops.back()->name;
			AddRoutText(stop_name1, routes_[i]->number, i_color);
			AddRoutText(stop_name2, routes_[i]->number, i_color);
		}
	}
}

void MapRenderer::SetCircleStop() {
	for (const auto& stop : stops_) {
		svg::Circle circle;
		circle.SetCenter(*stop_to_point_.at(stop))
			.SetRadius(settings_.stop_radius)
			.SetFillColor("white"s);
		map_objects_.Add(circle);
	}
}

void MapRenderer::AddStopText(const std::string_view& stop_name) {
	svg::Text text;
	svg::Text back_text;
	text.SetPosition(*stop_to_point_.at(stop_name))
		.SetOffset(settings_.stop_label_offset)
		.SetFontSize(settings_.stop_label_font_size)
		.SetFontFamily("Verdana"s)
		.SetData(static_cast<std::string>(stop_name))
		.SetFillColor("black"s);

	back_text.SetPosition(*stop_to_point_.at(stop_name))
		.SetOffset(settings_.stop_label_offset)
		.SetFontSize(settings_.stop_label_font_size)
		.SetFontFamily("Verdana"s)
		.SetData(static_cast<std::string>(stop_name))
		.SetFillColor(settings_.underlayer_color)
		.SetStrokeColor(settings_.underlayer_color)
		.SetStrokeWidth(settings_.underlayer_width)
		.SetStrokeLineCap(svg::StrokeLineCap::ROUND)
		.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

	map_objects_.Add(back_text);
	map_objects_.Add(text);
}

void MapRenderer::SetTextStop() {
	for (const auto& stop : stops_) {
		AddStopText(stop);
	}
}

void MapRenderer::SetRender() {
	SetPolyline();
	SetRoutes();
	SetCircleStop();
	SetTextStop();
}

svg::Document MapRenderer::GetDocument() {
	return std::move(map_objects_);
}

const MapSetting& MapRenderer::GetSetting() const {
	return settings_;
}

}