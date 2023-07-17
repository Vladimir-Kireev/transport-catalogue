#include "request_handler.h"

RequestHandler::RequestHandler(const catalogue::TransportCatalogue& db, renderer::MapRenderer& renderer) : db_(db), renderer_(renderer){
}

// Возвращает информацию о маршруте (запрос Bus)
std::optional<BusInformation> RequestHandler::GetBusStat(const std::string_view& bus_name) const {
	const auto& bus = db_.FindBus(bus_name);
	if (!bus) {
		return std::nullopt;
	}
	return db_.GetBusInfo(bus.value());
}

// Возвращает маршруты, проходящие через
const std::set<const Bus*, catalogue::Cmp>* RequestHandler::GetBusesByStop(const std::string_view& stop_name) const {
	const auto& stop = db_.FindStop(stop_name);
	if (!stop) {
		return nullptr;
	}
	return db_.GetStopInfo(stop.value());
}

svg::Document RequestHandler::RenderMap() const {
	renderer_.SetRender();
	return renderer_.GetDocument();
}