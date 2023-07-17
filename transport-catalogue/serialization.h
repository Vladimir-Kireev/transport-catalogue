#pragma once

#include "transport_catalogue.h"
#include "map_renderer.h"
#include "transport_router.h"

#include <string>
#include <fstream>

#include <transport_catalogue.pb.h>
#include <map_renderer.pb.h>

namespace serialization {

class Serializer {
private:
	using Graph = graph::DirectedWeightedGraph<double>;
	using Router = graph::Router<double>;

public:
	Serializer(catalogue::TransportCatalogue& catalogue, renderer::MapRenderer& renderer, TransportRouter& transport_router);

	void Serialize(std::string& filename);
	void Deserialize(std::string& filename);

private:
	void SerializeStops();
	void SerializeBuses();
	void SerializeDistance();
	void SerializeRenderer();
	void SerializeTranportRouter();
	void SerializeRoutSetting(tcatalogue_serialize::TransportRouter& transport_router);
	void SerializeIdToBus(tcatalogue_serialize::TransportRouter& transport_router);
	void SerializeGraph(tcatalogue_serialize::TransportRouter& transport_router);
	void SerializeRouter(tcatalogue_serialize::TransportRouter& transport_router);
	tcatalogue_serialize::Stop CreateStop(const Stop* stop);
	tcatalogue_serialize::Point CreatePoint(const svg::Point& point);
	tcatalogue_serialize::Color CreateColor(const svg::Color& color);

	svg::Color GetColor(const tcatalogue_serialize::Color& color);
	void DeserializeTransportCatalogue();
	void DeserializeMapRenderer();
	void DeserializeGraph();
	void DeserializeRouter();
	void DeserializeIdToBus();
	void DeserializeTransportRouter();


	catalogue::TransportCatalogue& catalogue_;
	renderer::MapRenderer& renderer_;
	TransportRouter& router_;
	tcatalogue_serialize::TransportCatalogue proto_catalogue_;
};

} // namespace serialization