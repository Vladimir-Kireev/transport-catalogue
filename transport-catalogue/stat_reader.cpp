#include <utility>

#include "stat_reader.h"

using namespace std::literals;

std::ostream& operator<<(std::ostream& out, const catalogue::BusInformation& iformation) {
    out << "Bus "s << iformation.number << ": "s
        << iformation.stops << " stops on route, "s
        << iformation.uniq_stops << " unique stops, "s
        << iformation.route_length << " route length, "s
        << static_cast<double>(iformation.route_length) / iformation.distance << " curvature"s;
    return out;
}

std::ostream& operator<<(std::ostream& out, const std::set<const catalogue::Bus*, catalogue::Cmp> buses) {
    out << "buses "s;
    for (const auto bus : buses) {
        out << bus->number << " "s;
    }
    return out;
}

namespace output {
Reader::Reader(catalogue::TransportCatalogue& catalogue) : catalogue_(catalogue) {}

void Reader::RequestBus(const std::string& bus_number) {
    auto bus = catalogue_.FindBus(bus_number);
    if (bus == std::nullopt) {
        std::cout << "Bus "s << bus_number << ": not found"s << std::endl;
        return;
    }
    std::cout << catalogue_.GetBusInfo(*bus.value()) << std::endl;
}

void Reader::RequestStop(const std::string& stop_name) {
    const auto stop = catalogue_.FindStop(stop_name);
    if (stop == std::nullopt) {
        std::cout << "Stop "s << stop_name << ": not found"s << std::endl;
        return;
    }
    const auto stop_info = catalogue_.GetStopInfo(stop.value());
    if (stop_info.empty()) {
        std::cout << "Stop "s << stop_name << ": no buses"s << std::endl;
        return;
    }
    std::cout << "Stop "s << stop_name << ": " << stop_info << std::endl;
}

void Reader::Read() {
    std::string line = std::move(ReadLine());
    std::string_view v_line = line;
    if (v_line.find("Bus") != std::string::npos) {
        v_line.remove_prefix(v_line.find_first_not_of("Bus"));
        RequestBus(DeleteSpace(v_line));
        return;
    }

    v_line.remove_prefix(v_line.find_first_not_of("Stop"));
    RequestStop(DeleteSpace(v_line));
}
}