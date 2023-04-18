#pragma once

#include <unordered_map>
#include <deque>
#include <string>
#include <string_view>
#include <vector>
#include <optional>
#include <set>

namespace catalogue{
struct Stop {
    std::string name;
    double latitude;
    double longitude;
};

struct Bus {
    std::string number;
    std::vector<const Stop*> stops;
    bool ring;  // one Ring to rule them all
};

struct BusInformation {
    std::string number;
    int stops;
    int uniq_stops;
    double distance;
    int route_length;
};

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
    void AddStop(const Stop& stop);
    void AddBus(const Bus& bus);
    void AddStopData(const std::string& stop, const std::string& next_stop, const int di);
    
    std::optional<const Stop*> FindStop(const std::string& stop) const;
    std::optional<const Bus*> FindBus(const std::string& bus) const;
    
    const BusInformation GetBusInfo(const Bus& bus);
    const std::set<const Bus*, Cmp> GetStopInfo(const Stop* stop);
    
private:
    double FillDistance(const Stop* rhs, const Stop* lhs);
    
    std::deque<Stop> stops_;
    std::deque<Bus> buses_;
    std::unordered_map<std::string_view, const Stop*> stopname_to_stop_;
    std::unordered_map<std::string_view, const Bus*> busname_to_bus_;
    std::unordered_map<const Stop*, std::set<const Bus*, Cmp>> stop_to_buses_;  // подумал, что тут Bus может пригодиться дальше. Достаточно хранить только названия?
    std::unordered_map<std::pair<const Stop*, const Stop*>, std::pair<double, int>, DistHasher> distanses_;
};
}