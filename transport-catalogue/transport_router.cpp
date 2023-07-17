#include "transport_router.h"

TransportRouter::TransportRouter(const catalogue::TransportCatalogue& t_catalogue) : t_catalogue_(t_catalogue){
}

void TransportRouter::SetTransportCatalogue(const catalogue::TransportCatalogue& t_catalogue) {
	if (&t_catalogue_ != &t_catalogue) {
		const_cast<catalogue::TransportCatalogue&>(t_catalogue_) = t_catalogue;
	}
}

void TransportRouter::SetSettingRouter(const SettingRouter setting) {
	setting_ = std::move(setting);
}

void TransportRouter::SetGraph(std::unique_ptr<Graph> graph) {
	graph_ = std::move(graph);
}

void TransportRouter::SetRouter(Router::RoutesInternalData&& routes_internal_data) {
	router_ = std::make_unique<graph::Router<double>>(*graph_.get(), std::move(routes_internal_data));
}

void TransportRouter::SetIdToBus(std::unordered_map<graph::EdgeId, EdgeIdInfo>&& edge_id_to_bus) {
	edge_id_to_bus_ = std::move(edge_id_to_bus);
	SetIdToStop();
}

void TransportRouter::SetIdToStop() {
	graph::VertexId vertex_id = 0;
	for (const auto& stop : t_catalogue_.GetStops()) {
		AddVertex(&stop, vertex_id);
		++vertex_id;
	}
}

void TransportRouter::CreateGraph() {
	const auto& stops = t_catalogue_.GetStops();
	graph::VertexId vertex_id = 0;
	for (const auto& stop : stops) {
		AddVertex(&stop, vertex_id);
		++vertex_id;
	}

	graph_ = std::make_unique<Graph>(stops.size());

	const auto& buses = t_catalogue_.GetBuses();
	for (const auto& bus : buses) {
		AddEdges(bus.stops, bus);
	}

	router_ = std::make_unique<graph::Router<double>>(*graph_.get());
}

void TransportRouter::AddVertex(const Stop* stop, graph::VertexId vertex_id) {
	id_to_stop_[vertex_id] = stop;
	stop_to_id_[stop] = vertex_id;
}

void TransportRouter::AddEdges(const std::vector<const Stop*> stops, const Bus& bus) {
	for (size_t i = 0; i < stops.size() - 1; ++i) {
		double weight = setting_.bus_wait_time;
		double weight_reverse = setting_.bus_wait_time;
		int stop_count = 0;
		const Stop* stop_begin = stops[i];

		for (size_t j = i + 1; j < stops.size(); ++j) {
			++stop_count;
			weight += CalculateDistance(stop_begin, stops[j]);
			graph::Edge<double> edge = { stop_to_id_.at(stops[i]), stop_to_id_.at(stops[j]), weight };
			graph::EdgeId id = graph_->AddEdge(edge);
			edge_id_to_bus_[id] = { &bus, stop_count };

			//заполняем обратную сторону
			if (!bus.is_ring) {
				weight_reverse += CalculateDistance(stops[j], stop_begin);
				graph::Edge<double> edge_reverse = { stop_to_id_.at(stops[j]), stop_to_id_.at(stops[i]), weight_reverse };
				graph::EdgeId id_reverse = graph_->AddEdge(edge_reverse);
				edge_id_to_bus_[id_reverse] = { &bus, stop_count };
			}

			stop_begin = stops[j];
		}
	}
}

std::optional<ResultRouters> TransportRouter::GetRoute(const std::string& first_stop, const std::string& second_stop) {
	const auto first_stop_ctl = t_catalogue_.FindStop(first_stop);
	const auto second_stop_ctl = t_catalogue_.FindStop(second_stop);
	if (!first_stop_ctl || !second_stop_ctl) {
		return std::nullopt;
	}

	std::optional<RouteInfo> rout_info = router_->BuildRoute(stop_to_id_.at(*first_stop_ctl), stop_to_id_.at(*second_stop_ctl));
	if (!rout_info.has_value()) {
		return std::nullopt;
	}

	ResultRouters result;
	result.total_time = rout_info->weight;
	for (const auto& edge_id : rout_info->edges) {
		const graph::Edge<double>& edge = graph_->GetEdge(edge_id);
		RoutersInfo routers_info = { id_to_stop_.at(edge.from),
			edge_id_to_bus_.at(edge_id).bus,
			edge_id_to_bus_.at(edge_id).stop_count,
			(edge.weight - setting_.bus_wait_time),
			setting_.bus_wait_time};
		result.info.push_back(routers_info);
	}

	return result;
}

double TransportRouter::CalculateDistance(const Stop* start, const Stop* end) {
	return (static_cast<double>(t_catalogue_.GetDistance(start, end)) * constants_.MINUTS_IN_HOUR) /
		(setting_.bus_velocity * constants_.METERS_IN_KLMTR);
}

const SettingRouter& TransportRouter::GetSetting() const {
	return setting_;
}

const TransportRouter::Graph* TransportRouter::GetGraph() const {
	return graph_.get();
}

const graph::Router<double>* TransportRouter::GetRouter() const {
	return router_.get();
}

const std::unordered_map<graph::EdgeId, EdgeIdInfo>& TransportRouter::GetIdToBus() const {
	return edge_id_to_bus_;
}