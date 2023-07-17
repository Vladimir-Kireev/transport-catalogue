#pragma once

#include "json.h"
#include "transport_catalogue.h"
#include "request_handler.h"
#include "map_renderer.h"
#include "transport_router.h"
#include "serialization.h"

#include <string>
#include <string_view>

namespace reader {
struct Cmp {
	bool operator()(const Bus* lhs, const Bus* rhs) const {
		return lhs->number < rhs->number;
	}
};

class JsonReader {
public:
	JsonReader(std::istream& input, renderer::MapRenderer& renderer, QueryType type);
	JsonReader(const JsonReader& other) = delete;
	JsonReader& operator=(const JsonReader& other) = delete;

	json::Document Load(std::istream& input);

	void GetCatalogue();
	void GetRouter();
	void OutQuery(std::ostream& output);

	void GetMap();
	void OutMap(std::ostream& output);

	void Serialize();
	void Deserialize();

private:
	const Bus MakeBus(const json::Dict& bus);
	const Stop MakeStop(const json::Dict& stop);
	void MakeDistanses(const std::string& stop, const json::Dict& stops_dist);
	void MakeStopToBuses(const std::string& stop);

	json::Dict GetBusAnswer(const json::Dict& bus);
	json::Dict GetStopAnswer(const json::Dict& stop);
	json::Dict GetMapAnswer(const json::Dict& map);
	json::Dict GetRoutAnswer(const json::Dict& route);

	renderer::MapSetting GetSetting() const;
	svg::Color GetColor(const json::Node& node_color) const;

	renderer::MapRenderer& renderer_;
	json::Document documents_;
	catalogue::TransportCatalogue catalogue_;
	RequestHandler handler_{ catalogue_,  renderer_ };
	TransportRouter transport_router_;
};
}