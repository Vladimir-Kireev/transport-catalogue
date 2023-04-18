#include <utility>

#include "input_reader.h"
#include "stat_reader.h"
#include "transport_catalogue.h"

int main() {
    catalogue::TransportCatalogue catalogue;

    {
        input::Reader reader(catalogue);
        const int input_request = ReadLineWithNumber();
        for (size_t i = 0; i < input_request; ++i) {
            reader.Load();
        }
        reader.GetCatalogue();
    }

    {
        output::Reader reader(catalogue);
        const int out_request = ReadLineWithNumber();
        for (size_t i = 0; i < out_request; ++i) {
            reader.Read();
        }
    }

    return 0;
}