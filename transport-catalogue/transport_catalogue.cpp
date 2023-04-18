#include <utility>
#include <algorithm>
#include <unordered_set>
#include <algorithm>
#include <string>

#include "transport_catalogue.h"
#include "geo.h"


namespace catalogue{
void TransportCatalogue::AddStop(const Stop& stop) {
    stops_.push_back(stop);
    stopname_to_stop_[stops_.back().name] = &stops_.back();
}

void TransportCatalogue::AddBus(const Bus& bus) {
    buses_.push_back(bus);
    busname_to_bus_[buses_.back().number] = &buses_.back();
}

void TransportCatalogue::AddStopData(const std::string& stop, const std::string& next_stop, const int di){
    const Stop* stop1 = stopname_to_stop_.at(stop);
    const Stop* stop2 = stopname_to_stop_.at(next_stop);
    double dist = FillDistance(stop1, stop2);
    distanses_[std::make_pair(stop1, stop2)] = std::make_pair(dist, di);
}


std::optional<const Stop*> TransportCatalogue::FindStop(const std::string& stop) const {
    if(stopname_to_stop_.find(stop) == stopname_to_stop_.end()){
        return std::nullopt;
    }
    return stopname_to_stop_.at(stop);
}

std::optional<const Bus*> TransportCatalogue::FindBus(const std::string& bus) const {
    if(busname_to_bus_.find(bus) == busname_to_bus_.end()){
        return std::nullopt;
    }
    return busname_to_bus_.at(bus);
}

double TransportCatalogue::FillDistance(const Stop* rhs, const Stop* lhs) {
    const Coordinates coordinates_rhs = {(*rhs).latitude, (*rhs).longitude};
    const Coordinates coordinates_lhs = {(*lhs).latitude, (*lhs).longitude};
    return ComputeDistance(coordinates_rhs, coordinates_lhs);
}

const BusInformation TransportCatalogue::GetBusInfo(const Bus& bus) {
    BusInformation result;
    result.number = bus.number;
    
    const std::unordered_set<const Stop*> set_stops(bus.stops.begin(), bus.stops.end());
    result.uniq_stops = set_stops.size();
    
    double all_dist = 0.0;
    int route_length = 0;
    for(size_t i = 0, j = 1; j < bus.stops.size(); ++i, ++j) {
        std::pair<const Stop*, const Stop*> key = std::make_pair(bus.stops[i], bus.stops[j]);
        if(distanses_.find(key) == distanses_.end()){
            key = std::make_pair(bus.stops[j], bus.stops[i]);
        }
        all_dist += distanses_[key].first;
        route_length += distanses_[key].second;
        if(!bus.ring){
            std::pair<const Stop*, const Stop*> reverse_key = std::make_pair(bus.stops[j], bus.stops[i]);
            route_length += distanses_.find(reverse_key) != distanses_.end() ? distanses_[reverse_key].second : distanses_[key].second;
        }
    }
    if(bus.ring) {
        all_dist += FillDistance(bus.stops.front(), bus.stops.back());
        result.stops = bus.stops.size();
    } else {
        all_dist *= 2;
        result.stops = bus.stops.size() * 2 - 1;
    }
    result.distance = all_dist;
    result.route_length = route_length;
    return result;
}
    
// Не уверен в методе. Не было ограничений по сложности. Но, кажется, лучше заполнять контейнер stop_to_buses_ на вводе?
const std::set<const Bus*, Cmp> TransportCatalogue::GetStopInfo(const Stop* stop){
    if(stop_to_buses_.find(stop) != stop_to_buses_.end()){
        return stop_to_buses_.at(stop);
    }
    std::set<const Bus*, Cmp> result;
    for(const Bus& bus : buses_){
        if(std::find(bus.stops.begin(), bus.stops.end(), stop) != bus.stops.end()){
            result.insert(&bus);
        }
    }
    stop_to_buses_[stop] = result;
    
    return result;
}
}