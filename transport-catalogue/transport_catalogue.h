#pragma once
#include "domain.h"

#include <unordered_map>
#include <unordered_set>
#include <deque>
#include <string>
#include <string_view>
#include <vector>
#include <optional>
#include <set>

namespace catalogue{

struct DistHasher {
    size_t operator()(const std::pair<const Stop*, const Stop*> key) const {
        return hasher_(key.first)*37 + hasher_(key.second);
    }
    
private:
    std::hash<const void*> hasher_;
};

struct Cmp{
    bool operator()(const Bus* lhs, const Bus* rhs) const{
        return lhs->number < rhs->number;
    }
};

class TransportCatalogue {
public:
    using AllDistance = std::unordered_map<std::pair<const Stop*, const Stop*>, std::pair<double, int>, DistHasher>;

    const Stop* AddStop(const Stop& stop);
    const Bus* AddBus(const Bus& bus);
    void SetDistance(const std::string& stop, const std::string& next_stop, const int actual_dist);
    void SetStopToBuses(const std::string& base_stop);
    
    std::optional<const Stop*> FindStop(const std::string_view& stop) const;
    std::optional<const Bus*> FindBus(const std::string_view& bus) const;
    
    const BusInformation GetBusInfo(const Bus* bus) const;
    const std::set<const Bus*, Cmp>* GetStopInfo(const Stop* stop) const;
    double GetDistance(const Stop* first, const Stop* second) const;
    const std::deque<Stop>& GetStops() const;
    const std::deque<Bus>& GetBuses() const;
    const AllDistance& GetAllDistance() const;
    
private:
    
    std::deque<Stop> stops_;
    std::deque<Bus> buses_;
    std::unordered_map<std::string_view, const Stop*> stopname_to_stop_;
    std::unordered_map<std::string_view, const Bus*> busname_to_bus_;
    std::unordered_map<const Stop*, std::set<const Bus*, Cmp>> stop_to_buses_;
    AllDistance distanses_;
};
}