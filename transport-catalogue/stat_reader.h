#pragma once
#include <iostream>

#include "input_reader.h"
#include "transport_catalogue.h"

namespace output {
class Reader {
public:
    explicit Reader(catalogue::TransportCatalogue& catalogue);
    Reader(const Reader& other) = delete;

    void RequestBus(const std::string& bus_number);
    void RequestStop(const std::string& stop_name);

    void Read();

private:
    catalogue::TransportCatalogue& catalogue_;
};
}