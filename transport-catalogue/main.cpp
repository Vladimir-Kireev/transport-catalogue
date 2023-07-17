#include "json_reader.h"

#include <fstream>
#include <iostream>
#include <string_view>

using namespace std::literals;

void PrintUsage(std::ostream& stream = std::cerr) {
    stream << "Usage: transport_catalogue [make_base|process_requests]\n"sv;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        PrintUsage();
        return 1;
    }

    const std::string_view mode(argv[1]);

    if (mode == "make_base"sv) {
        renderer::MapRenderer renderer{};
        reader::JsonReader read_ctlg{ std::cin, renderer, QueryType::MakeBase };
    } else if (mode == "process_requests"sv) {
        renderer::MapRenderer renderer{};
        reader::JsonReader read_ctlg{ std::cin, renderer, QueryType::ProcessRequests };
        read_ctlg.OutQuery(std::cout);
    } else {
        PrintUsage();
        return 1;
    }
}