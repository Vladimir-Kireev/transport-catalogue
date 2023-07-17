#pragma once
#include "geo.h"


#include <vector>
#include <string>

struct Stop {
    std::string name;
    geo::Coordinates coordinates{};
};

struct Bus {
    std::string number;
    std::vector<const Stop*> stops{};
    bool is_ring;  // one Ring to rule them all
};

struct BusInformation {
    std::string number;
    int stops;
    int uniq_stops;
    double distance;
    int route_length;
};

enum class QueryType { MakeBase, ProcessRequests };