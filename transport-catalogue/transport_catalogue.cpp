#include "transport_catalogue.h"

#include <utility>
#include <algorithm>
#include <unordered_set>
#include <algorithm>
#include <string>
#include <iostream>

namespace catalogue{
const Stop* TransportCatalogue::AddStop(const Stop& stop) {
    stops_.push_back(stop);
    stopname_to_stop_[stops_.back().name] = &stops_.back();
    return &stops_.back();
}

const Bus* TransportCatalogue::AddBus(const Bus& bus) {
    buses_.push_back(bus);
    busname_to_bus_[buses_.back().number] = &buses_.back();
    return &buses_.back();
}

void TransportCatalogue::SetDistance(const std::string& stop, const std::string& next_stop, const int actual_dist){
    const Stop* stop1 = stopname_to_stop_.at(stop);
    const Stop* stop2 = stopname_to_stop_.at(next_stop);
    double dist = geo::ComputeDistance(stop1->coordinates, stop2->coordinates);
    distanses_[std::make_pair(stop1, stop2)] = std::make_pair(dist, actual_dist);
}

void TransportCatalogue::SetStopToBuses(const std::string& base_stop) {
    const Stop* stop = stopname_to_stop_.at(base_stop);
    stop_to_buses_[stop] = {};
    for (const auto& bus : buses_) {
        if (std::find(bus.stops.begin(), bus.stops.end(), stop) != bus.stops.end()) {
            stop_to_buses_[stop].insert(&bus);
        }
    }
}

std::optional<const Stop*> TransportCatalogue::FindStop(const std::string_view& stop) const {
    if(stopname_to_stop_.find(stop) == stopname_to_stop_.end()){
        return std::nullopt;
    }
    return stopname_to_stop_.at(stop);
}

std::optional<const Bus*> TransportCatalogue::FindBus(const std::string_view& bus) const {
    if(busname_to_bus_.find(bus) == busname_to_bus_.end()){
        return std::nullopt;
    }
    return busname_to_bus_.at(bus);
}

const BusInformation TransportCatalogue::GetBusInfo(const Bus* bus) const {
    BusInformation result;
    result.number = bus->number;
    
    const std::unordered_set<const Stop*> set_stops(bus->stops.begin(), bus->stops.end());
    result.uniq_stops = set_stops.size();
    
    double all_dist = 0.0;
    int route_length = 0;
    for(size_t i = 0, j = 1; j < bus->stops.size(); ++i, ++j) {
        std::pair<const Stop*, const Stop*> key = std::make_pair(bus->stops[i], bus->stops[j]);
        if(distanses_.find(key) == distanses_.end()){
            key = std::make_pair(bus->stops[j], bus->stops[i]);
        }
        all_dist += distanses_.at(key).first;
        route_length += distanses_.at(key).second;
        if(!bus->is_ring){
            std::pair<const Stop*, const Stop*> reverse_key = std::make_pair(bus->stops[j], bus->stops[i]);
            route_length += distanses_.find(reverse_key) != distanses_.end() ? distanses_.at(reverse_key).second : distanses_.at(key).second;
        }
    }
    if(bus->is_ring) {
        all_dist += geo::ComputeDistance(bus->stops.front()->coordinates, bus->stops.back()->coordinates);
        result.stops = bus->stops.size();
    } else {
        all_dist *= 2;
        result.stops = bus->stops.size() * 2 - 1;
    }
    result.distance = all_dist;
    result.route_length = route_length;
    return result;
}

const std::set<const Bus*, Cmp>* TransportCatalogue::GetStopInfo(const Stop* stop) const{
    return &stop_to_buses_.at(stop);
}

double TransportCatalogue::GetDistance(const Stop* first, const Stop* second) const {
    std::pair<const Stop*, const Stop*> key = std::make_pair(first, second);
    if (distanses_.find(key) == distanses_.end()) {
        key = std::make_pair(second, first);
    }
    return static_cast<double>(distanses_.at(key).second);
}

const std::deque<Stop>& TransportCatalogue::GetStops() const {
    return stops_;
}

const std::deque<Bus>& TransportCatalogue::GetBuses() const {
    return buses_;
}

const TransportCatalogue::AllDistance& TransportCatalogue::GetAllDistance() const {
    return distanses_;
}

}