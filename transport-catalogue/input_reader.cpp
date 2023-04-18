#include "input_reader.h"

std::string ReadLine() {
    std::string s;
    std::getline(std::cin, s);
    return s;
}

int ReadLineWithNumber() {
    int result;
    std::cin >> result;
    ReadLine();
    return result;
}

std::string DeleteSpace(std::string_view str_view) {
    str_view.remove_prefix(str_view.find_first_not_of(" "));
    str_view.remove_suffix(str_view.size() - (str_view.find_last_not_of(" ") + 1));
    return static_cast<std::string>(str_view);
}

namespace input {
Reader::Reader(catalogue::TransportCatalogue& object) : object_(object) {
}

void Reader::Load() {
    std::string line = ReadLine();
    std::string_view v_line = line;
    v_line.remove_prefix(v_line.find_first_not_of(" "));
    if (v_line.find("Bus") != std::string::npos) {
        v_line.remove_prefix(v_line.find_first_not_of("Bus"));
        routes_.push_back(std::move(static_cast<std::string>(v_line)));
        return;
    }
    v_line.remove_prefix(v_line.find_first_not_of("Stop"));
    object_.AddStop(MakeStop(v_line));
}

const catalogue::Stop Reader::MakeStop(std::string_view stop) {
    catalogue::Stop transport_stop;
    transport_stop.name = std::move(GetPartRequest(stop, ':'));

    transport_stop.latitude = std::stod(std::move(GetPartRequest(stop, ',')));
    transport_stop.longitude = std::stod(std::move(GetPartRequest(stop, ',')));

    if (stop.find('.') == std::string_view::npos) {
        auto separator = stop.find(',');
        StopData stop_data;
        stop_data.stop = transport_stop.name;
        while (separator != std::string_view::npos) {
            int distance = std::stoi(std::move(GetPartRequest(stop, 'm')));
            std::string stop_name = std::move(GetPartRequest(stop, ','));
            stop_data.di_stop.push_back(std::make_pair(distance, stop_name));
            separator = stop.find(',');
        }
        int distance = std::stoi(std::move(GetPartRequest(stop, 'm')));
        std::string stop_name = std::move(DeleteSpace(stop));
        stop_data.di_stop.push_back(std::make_pair(distance, stop_name));
        stops_datas_.push_back(stop_data);
    }
    return transport_stop;
}

std::string Reader::GetPartRequest(std::string_view& request, const char separator) {
    const auto sep_iter = request.find(separator);
    std::string_view result = request.substr(0, sep_iter);
    if (sep_iter != std::string_view::npos && separator == ',') {
        request.remove_prefix(sep_iter + 1);
    }
    else if (separator == 'm') {
        request.remove_prefix(request.find("to"sv) + 2);
    }
    else if (separator == ':') {
        request.remove_prefix(sep_iter + 1);
    }
    return DeleteSpace(result);
}

const catalogue::Bus Reader::MakeBus(std::string_view bus) {
    catalogue::Bus transport_bus;
    std::string_view bus_number = bus.substr(0, bus.find(':'));
    transport_bus.number = std::move(DeleteSpace(bus_number));
    bus.remove_prefix(bus.find(':') + 1);

    char separator;
    if (bus.find('>') != std::string_view::npos) {
        separator = '>';
        transport_bus.ring = true;
    }
    else {
        separator = '-';
        transport_bus.ring = false;
    }

    while (!bus.empty()) {
        std::string_view bus_station = bus.substr(0, bus.find(separator));
        transport_bus.stops.push_back(object_.FindStop(DeleteSpace(bus_station)).value());

        if (bus.find(separator) == bus.npos) {
            break;
        }
        bus.remove_prefix(bus.find(separator) + 1);
    }
    return transport_bus;
}

void Reader::GetCatalogue() {
    for (auto& rout : routes_) {
        object_.AddBus(MakeBus(rout));
    }
    for (const auto& stop_data : stops_datas_) {
        for (const auto& data : stop_data.di_stop) {
            object_.AddStopData(stop_data.stop, data.second, data.first);
        }
    }
}
}