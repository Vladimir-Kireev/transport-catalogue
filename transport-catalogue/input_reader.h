#pragma once

#include <vector>
#include <unordered_map>
#include <string>
#include <string_view>
#include <iostream>

#include "transport_catalogue.h"

using namespace std::literals;

std::string ReadLine();
int ReadLineWithNumber();
std::string DeleteSpace(std::string_view str_view);

namespace input{
struct StopData{
    std::string stop;
    std::vector<std::pair<int, std::string>> di_stop; 
};

class Reader {
public:
    explicit Reader(catalogue::TransportCatalogue& object);
    Reader(const Reader& other) = delete;
    
    void Load();
    
    const catalogue::Bus MakeBus(std::string_view bus);
    const catalogue::Stop MakeStop(std::string_view stop);
    void GetCatalogue();
    
private:
    std::string GetPartRequest(std::string_view& request, const char separator);
    
    catalogue::TransportCatalogue& object_;
    std::vector<std::string> routes_;
    std::vector<StopData> stops_datas_;
};
}