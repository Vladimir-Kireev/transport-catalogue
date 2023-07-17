#pragma once
#include "transport_catalogue.h"
#include "map_renderer.h"

#include <string_view>

class RequestHandler {
public:
    RequestHandler(const catalogue::TransportCatalogue& db, renderer::MapRenderer& renderer);

    // Возвращает информацию о маршруте (запрос Bus)
    std::optional<BusInformation> GetBusStat(const std::string_view& bus_name) const;

    // Возвращает маршруты, проходящие через
    const std::set<const Bus*, catalogue::Cmp>* GetBusesByStop(const std::string_view& stop_name) const;

    svg::Document RenderMap() const;

private:
    // RequestHandler использует агрегацию объектов "Транспортный Справочник" и "Визуализатор Карты"
    const catalogue::TransportCatalogue& db_;
    renderer::MapRenderer& renderer_;
};
