#include "serialization.h"

#include <algorithm>

namespace serialization {

Serializer::Serializer(catalogue::TransportCatalogue& catalogue, renderer::MapRenderer& renderer, TransportRouter& transport_router) :
	catalogue_(catalogue),
	renderer_(renderer),
	router_(transport_router){
}

void Serializer::Serialize(std::string& filename) {
	std::ofstream out_file(filename, std::ios::binary);

	SerializeStops();
	SerializeBuses();
	SerializeDistance();
	SerializeRenderer();
	SerializeTranportRouter();

	proto_catalogue_.SerializeToOstream(&out_file);
}

void Serializer::Deserialize(std::string& filename) {
	std::ifstream in_file(filename, std::ios::binary);
	proto_catalogue_.ParseFromIstream(&in_file);

	DeserializeTransportCatalogue();
	DeserializeMapRenderer();
	DeserializeTransportRouter();
}


void Serializer::SerializeStops() {
	for (const auto& stop : catalogue_.GetStops()) {
		*proto_catalogue_.add_stops() = std::move(CreateStop(&stop));
	}
}

void Serializer::SerializeBuses() {
	for (const auto& bus : catalogue_.GetBuses()) {
		tcatalogue_serialize::Bus new_bus;
		new_bus.set_number(bus.number);
		new_bus.set_is_ring(bus.is_ring);

		for (const auto& stop : bus.stops) {
			*new_bus.add_stops() = std::move(CreateStop(stop));
		}
		*proto_catalogue_.add_buses() = std::move(new_bus);
	}
}

void Serializer::SerializeDistance() {
	for (const auto& [stops, dists] : catalogue_.GetAllDistance()) {
		tcatalogue_serialize::Distance new_distance;
		new_distance.set_from(stops.first->name);
		new_distance.set_to(stops.second->name);
		new_distance.set_dist(dists.second);

		*proto_catalogue_.add_distanses() = std::move(new_distance);
	}
}

tcatalogue_serialize::Stop Serializer::CreateStop(const Stop* stop) {
	tcatalogue_serialize::Stop new_stop;
	tcatalogue_serialize::Coordinates new_coordinates;

	new_coordinates.set_lat(stop->coordinates.lat);
	new_coordinates.set_lng(stop->coordinates.lng);

	new_stop.set_name(stop->name);
	*new_stop.mutable_coords() = std::move(new_coordinates);

	return new_stop;
}

tcatalogue_serialize::Point Serializer::CreatePoint(const svg::Point& point) {
	tcatalogue_serialize::Point new_point;
	
	new_point.set_x(point.x);
	new_point.set_y(point.y);

	return new_point;
}

tcatalogue_serialize::Color Serializer::CreateColor(const svg::Color& color) {
	tcatalogue_serialize::Color new_color;

	if (const std::string* color_ptr = std::get_if<std::string>(&color)) {
		new_color.set_str_color(*color_ptr);
	}
	else if (const svg::Rgb* color_ptr = std::get_if<svg::Rgb>(&color)) {
		tcatalogue_serialize::Rgb new_rgb;

		new_rgb.set_red(color_ptr->red);
		new_rgb.set_green(color_ptr->green);
		new_rgb.set_blue(color_ptr->blue);

		*new_color.mutable_rgb() = std::move(new_rgb);
	}
	else if (const svg::Rgba* color_ptr = std::get_if<svg::Rgba>(&color)) {
		tcatalogue_serialize::Rgba new_rgba;

		new_rgba.set_red(color_ptr->rgb.red);
		new_rgba.set_green(color_ptr->rgb.green);
		new_rgba.set_blue(color_ptr->rgb.blue);
		new_rgba.set_opacity(color_ptr->opacity);

		*new_color.mutable_rgba() = std::move(new_rgba);
	}

	return new_color;
}

void Serializer::SerializeRenderer() {
	const renderer::MapSetting& setting = renderer_.GetSetting();
	tcatalogue_serialize::MapSetting srlz_setting;

	srlz_setting.set_width(setting.width);
	srlz_setting.set_height(setting.height);
	srlz_setting.set_padding(setting.padding);
	srlz_setting.set_line_width(setting.line_width);
	srlz_setting.set_stop_radius(setting.stop_radius);
	srlz_setting.set_bus_label_font_size(setting.bus_label_font_size);
	srlz_setting.set_stop_label_font_size(setting.stop_label_font_size);
	srlz_setting.set_underlayer_width(setting.underlayer_width);

	*srlz_setting.mutable_bus_label_offset() = std::move(CreatePoint(setting.bus_label_offset));
	*srlz_setting.mutable_stop_label_offset() = std::move(CreatePoint(setting.stop_label_offset));
	*srlz_setting.mutable_underlayer_color() = std::move(CreateColor(setting.underlayer_color));

	for (const auto& color : setting.color_palette) {
		*srlz_setting.add_color_palette() = std::move(CreateColor(color));
	}

	*proto_catalogue_.mutable_map_setting() = std::move(srlz_setting);
}

void Serializer::SerializeRoutSetting(tcatalogue_serialize::TransportRouter& transport_router) {
	const SettingRouter& rout_setting = router_.GetSetting();
	tcatalogue_serialize::SettingRouter setting;
	setting.set_bus_velocity(rout_setting.bus_velocity);
	setting.set_bus_wait_time(rout_setting.bus_wait_time);
	*transport_router.mutable_setting() = std::move(setting);
}

void Serializer::SerializeIdToBus(tcatalogue_serialize::TransportRouter& transport_router) {
	for (const auto& [key, value] : router_.GetIdToBus()) {
		tcatalogue_serialize::EdgeIdInfo info;
		info.set_bus(value.bus->number);
		info.set_stop_count(value.stop_count);
		info.set_key(key);
		*transport_router.add_edge_id_to_bus() = std::move(info);
	}
}

void Serializer::SerializeGraph(tcatalogue_serialize::TransportRouter& transport_router) {
	tcatalogue_serialize::Graph new_graph;
	for (const auto& edge : router_.GetGraph()->GetEdges()) {
		tcatalogue_serialize::Edge new_edge;
		new_edge.set_from(edge.from);
		new_edge.set_to(edge.to);
		new_edge.set_weight(edge.weight);

		*new_graph.add_edges() = std::move(new_edge);
	}

	for (size_t i = 0; i < router_.GetGraph()->GetIncidenceLists().size(); ++i) {
		tcatalogue_serialize::RepeatLists new_lists;
		for (const auto& list : router_.GetGraph()->GetIncidenceLists()[i]) {
			new_lists.add_lists(list);
		}
		*new_graph.add_incidence_lists() = std::move(new_lists);
	}
	*transport_router.mutable_graph() = std::move(new_graph);
}

void Serializer::SerializeRouter(tcatalogue_serialize::TransportRouter& transport_router) {
	tcatalogue_serialize::Router new_router;
	for (size_t i = 0; i < router_.GetRouter()->GetData().size(); ++i) {
		tcatalogue_serialize::RoutesInternalData datas;
		for (const auto& data : router_.GetRouter()->GetData()[i]) {
			tcatalogue_serialize::RouteInternalData new_data;
			if (data) {
				new_data.set_weight(data->weight);
				if (data->prev_edge) {
					new_data.set_prev_edge(*(data->prev_edge));
				}
				else {
					new_data.set_no_prev_edge(true);
				}
				new_data.set_is_no_data(false);
			}
			else {
				new_data.set_is_no_data(true);
			}
			*datas.add_datas() = std::move(new_data);
		}
		*new_router.add_routes_internal_data() = std::move(datas);
	}

	*transport_router.mutable_router() = std::move(new_router);
}

void Serializer::SerializeTranportRouter() {
	tcatalogue_serialize::TransportRouter transport_router;

	SerializeRoutSetting(transport_router);
	SerializeIdToBus(transport_router);
	SerializeGraph(transport_router);
	SerializeRouter(transport_router);

	*proto_catalogue_.mutable_transport_router() = std::move(transport_router);
}


void Serializer::DeserializeTransportCatalogue() {
	for (const auto& stop : proto_catalogue_.stops()) {
		const tcatalogue_serialize::Coordinates& coords = stop.coords();
		Stop new_stop;
		new_stop.name = stop.name();
		new_stop.coordinates = { coords.lat(), coords.lng() };

		catalogue_.AddStop(new_stop);
	}

	for (const auto& bus : proto_catalogue_.buses()) {
		Bus new_bus;
		new_bus.number = bus.number();
		new_bus.is_ring = bus.is_ring();
		for (const auto& stop : bus.stops()) {
			const Stop* ctlg_stop = catalogue_.FindStop(stop.name()).value();
			new_bus.stops.push_back(ctlg_stop);
		}
		catalogue_.AddBus(new_bus);
	}

	for (const auto& dist : proto_catalogue_.distanses()) {
		catalogue_.SetDistance(dist.from(), dist.to(), dist.dist());
	}

	for (const auto& stop : proto_catalogue_.stops()) {
		catalogue_.SetStopToBuses(stop.name());
	}
}

svg::Color Serializer::GetColor(const tcatalogue_serialize::Color& color) {
	svg::Color result;
	if (color.has_rgb()) {
		return svg::Rgb(color.rgb().red(), color.rgb().green(), color.rgb().blue());
	}
	else if (color.has_rgba()) {
		return svg::Rgba(color.rgba().red(), color.rgba().green(), color.rgba().blue(), color.rgba().opacity());
	}
	else if (color.str_color().size() != 0) {
		return svg::Color{color.str_color()};
	}
	return result;
}

void Serializer::DeserializeMapRenderer() {
	renderer::MapSetting setting;
	const tcatalogue_serialize::MapSetting& proto_setting = proto_catalogue_.map_setting();

	setting.width = proto_setting.width();
	setting.height = proto_setting.height();
	setting.padding = proto_setting.padding();
	setting.line_width = proto_setting.line_width();
	setting.stop_radius = proto_setting.stop_radius();
	setting.bus_label_font_size = proto_setting.bus_label_font_size();
	setting.stop_label_font_size = proto_setting.stop_label_font_size();
	setting.underlayer_width = proto_setting.underlayer_width();

	setting.bus_label_offset = { proto_setting.bus_label_offset().x(), proto_setting.bus_label_offset().y() };
	setting.stop_label_offset = { proto_setting.stop_label_offset().x(), proto_setting.stop_label_offset().y() };
	setting.underlayer_color = std::move(GetColor(proto_setting.underlayer_color()));

	for (const auto& color : proto_setting.color_palette()) {
		setting.color_palette.push_back(std::move(GetColor(color)));
	}

	renderer_.AddSetting(std::move(setting));
	renderer_.RenderCoords(catalogue_.GetBuses());
}

void Serializer::DeserializeGraph() {
	const tcatalogue_serialize::Graph& proto_graph = proto_catalogue_.transport_router().graph();

	std::vector<graph::Edge<double>> edges;
	edges.reserve(proto_graph.edges_size());
	for (const auto& edge : proto_graph.edges()) {
		edges.emplace_back(graph::Edge<double>{ edge.from(), edge.to(), edge.weight() });
	}

	std::vector<std::vector<graph::EdgeId>> incidence_lists;
	incidence_lists.resize(proto_graph.incidence_lists_size());
	for (size_t i = 0; i < proto_graph.incidence_lists_size(); ++i) {
		for (const auto& list : proto_graph.incidence_lists(i).lists()) {
			incidence_lists[i].push_back(list);
		}
	}
    
	std::unique_ptr<Graph> graph = std::make_unique<Graph>(std::move(edges), std::move(incidence_lists));
	router_.SetGraph(std::move(graph));
}

void Serializer::DeserializeRouter() {
	const tcatalogue_serialize::Router& proto_router = proto_catalogue_.transport_router().router();
	Router::RoutesInternalData routes_internal_data;
	routes_internal_data.resize(proto_router.routes_internal_data_size());
	for (size_t i = 0; i < proto_router.routes_internal_data_size(); ++i) {
		for (const auto& data : proto_router.routes_internal_data(i).datas()) {
			if (data.is_no_data()) {
				routes_internal_data[i].push_back(std::nullopt);
				continue;
			}

			Router::RouteInternalData route_internal_data;
			route_internal_data.weight = data.weight();
			if (data.no_prev_edge()) {
				route_internal_data.prev_edge = std::nullopt;
			}
			else {
				route_internal_data.prev_edge = data.prev_edge();
			}
			routes_internal_data[i].push_back(route_internal_data);
		}
	}

	router_.SetRouter(std::move(routes_internal_data));
}

void Serializer::DeserializeIdToBus() {
	std::unordered_map<graph::EdgeId, EdgeIdInfo> edge_id_to_bus;
	for (const auto& edge_id_info : proto_catalogue_.transport_router().edge_id_to_bus()) {
		edge_id_to_bus[edge_id_info.key()] = EdgeIdInfo{ 
			*catalogue_.FindBus(edge_id_info.bus())
			, static_cast<int>(edge_id_info.stop_count())};
	}

	router_.SetIdToBus(std::move(edge_id_to_bus));
}

void Serializer::DeserializeTransportRouter() {
	DeserializeGraph();
	DeserializeRouter();
	DeserializeIdToBus();

	const tcatalogue_serialize::SettingRouter& proto_setting = proto_catalogue_.transport_router().setting();
	SettingRouter setting = { static_cast<int>(proto_setting.bus_velocity()), static_cast<int>(proto_setting.bus_wait_time()) };

	router_.SetSettingRouter(setting);
	router_.SetTransportCatalogue(catalogue_);
}

} // namespace serialization