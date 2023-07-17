#pragma once
#include "router.h"
#include "graph.h"
#include "transport_catalogue.h"

#include <unordered_map>
#include <string>
#include <optional>
#include <vector>
#include <memory>
#include <set>

struct SettingRouter {
	int bus_velocity = 0;
	int bus_wait_time = 0;
};

struct Constants {
	inline static const int MINUTS_IN_HOUR = 60;
	inline static const int METERS_IN_KLMTR = 1000;
};

struct EdgeIdInfo {  // храним указатель на автобус и кол-во остановок для соотв. EdgeId
	const Bus* bus;
	int stop_count;
 };

struct RoutersInfo {
	const Stop* stop_name;
	const Bus* bus_name;
	int span_count;
	double time;
	int bus_wait_time;
};

struct ResultRouters {
	double total_time;
	std::vector<RoutersInfo> info;
};

class TransportRouter {
public:
	using Graph = graph::DirectedWeightedGraph<double>;
	using RouteInfo = graph::Router<double>::RouteInfo;
	using Router = graph::Router<double>;

	TransportRouter() = default;
	explicit TransportRouter(const catalogue::TransportCatalogue& t_catalogue);
	void SetTransportCatalogue(const catalogue::TransportCatalogue& t_catalogue);
	void SetSettingRouter(const SettingRouter setting);
	void SetGraph(std::unique_ptr<Graph> graph);
	void SetRouter(Router::RoutesInternalData&& routes_internal_data);
	void SetIdToBus(std::unordered_map<graph::EdgeId, EdgeIdInfo>&& edge_id_to_bus);
	void SetIdToStop();

	void CreateGraph();

	std::optional<ResultRouters> GetRoute(const std::string& first_stop, const std::string& second_stop);

	const SettingRouter& GetSetting() const;
	const Graph* GetGraph() const;
	const graph::Router<double>* GetRouter() const;
	const std::unordered_map<graph::EdgeId, EdgeIdInfo>& GetIdToBus() const;

private:
	void AddVertex(const Stop* stop, graph::VertexId vertex_id);
	void AddEdges(const std::vector<const Stop*> stops, const Bus& bus);
	double CalculateDistance(const Stop* start, const Stop* end);

	std::unordered_map<graph::VertexId, const Stop*> id_to_stop_;
	std::unordered_map<const Stop*, graph::VertexId> stop_to_id_;
	std::unordered_map<graph::EdgeId, EdgeIdInfo> edge_id_to_bus_;
	SettingRouter setting_;
	Constants constants_;
	const catalogue::TransportCatalogue& t_catalogue_;
	std::unique_ptr<Graph> graph_;
	std::unique_ptr<graph::Router<double>> router_;
};