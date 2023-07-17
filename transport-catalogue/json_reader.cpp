#include "json_reader.h"
#include "json_builder.h"

#include <iostream>
#include <sstream>

using namespace std::literals;

namespace reader {
	JsonReader::JsonReader(std::istream& input, renderer::MapRenderer& renderer, QueryType type) :
		renderer_(renderer),
		transport_router_(catalogue_)
	{
		documents_ = Load(input);
		if (type == QueryType::MakeBase) {
			GetCatalogue();
			GetRouter();
			GetMap();
			Serialize();
		}
		else if (type == QueryType::ProcessRequests) {
			Deserialize();
		}
	}

	json::Document JsonReader::Load(std::istream& input) {
		return json::Load(input);
	}

	const Bus JsonReader::MakeBus(const json::Dict& bus) {
		Bus transport_bus;
		transport_bus.number = bus.at("name"s).AsString();
		transport_bus.is_ring = bus.at("is_roundtrip"s).AsBool();
		for (const auto& stop : bus.at("stops"s).AsArray()) {
			const std::string str_stop = stop.AsString();
			transport_bus.stops.push_back(catalogue_.FindStop(str_stop).value());
		}
		return transport_bus;
	}

	const Stop JsonReader::MakeStop(const json::Dict& stop) {
		Stop transport_stop;
		transport_stop.name = stop.at("name"s).AsString();
		transport_stop.coordinates = { stop.at("latitude"s).AsDouble(), stop.at("longitude"s).AsDouble() };
		return transport_stop;
	}

	void JsonReader::MakeDistanses(const std::string& stop, const json::Dict& stops_dist) {
		for (const auto& [key, value] : stops_dist) {
			catalogue_.SetDistance(stop, key, value.AsInt());
		}
	}

	void JsonReader::MakeStopToBuses(const std::string& stop) {
		catalogue_.SetStopToBuses(stop);
	}

	void JsonReader::GetCatalogue() {
		const json::Array& input_document = documents_.GetRoot().AsDict().at("base_requests"s).AsArray();
		for (const auto& doc : input_document) {
			const json::Dict& doc_map = doc.AsDict();
			if (doc_map.at("type"s) == "Stop"s) {
				catalogue_.AddStop(MakeStop(doc_map));
			}
		}

		for (const auto& doc : input_document) {
			const json::Dict& doc_map = doc.AsDict();
			if (doc_map.at("type"s) == "Bus"s) {
				const Bus* bus_in_ctlg = catalogue_.AddBus(MakeBus(doc_map));
			}
		}
		for (const auto& doc : input_document) {
			const json::Dict& doc_map = doc.AsDict();
			if (doc_map.at("type"s) == "Stop"s) {
				MakeStopToBuses(doc_map.at("name"s).AsString());
				MakeDistanses(doc_map.at("name"s).AsString(), doc_map.at("road_distances"s).AsDict());
			}
		}
	}

	void JsonReader::GetRouter() {
		const json::Dict& input_document = documents_.GetRoot().AsDict().at("routing_settings"s).AsDict();
		SettingRouter setting{ input_document.at("bus_velocity"s).AsInt(), input_document.at("bus_wait_time"s).AsInt(), };
		transport_router_.SetSettingRouter(std::move(setting));
		transport_router_.CreateGraph();
	}

	json::Dict JsonReader::GetBusAnswer(const json::Dict& bus) {
		const auto& bus_info = handler_.GetBusStat(bus.at("name"s).AsString());

		if (!bus_info) {
			return json::Builder{}.StartDict()
				.Key("request_id"s).Value(bus.at("id"s).AsDouble())
				.Key("error_message"s).Value("not found"s)
				.EndDict().Build().AsDict();
		}
		return json::Builder{}.StartDict()
			.Key("request_id"s).Value(bus.at("id"s).AsDouble())
			.Key("route_length"s).Value(bus_info->route_length)
			.Key("stop_count"s).Value(bus_info->stops)
			.Key("unique_stop_count"s).Value(bus_info->uniq_stops)
			.Key("curvature"s).Value(static_cast<double>(bus_info->route_length) / bus_info->distance)
			.EndDict().Build().AsDict();
	}

	json::Dict JsonReader::GetStopAnswer(const json::Dict& stop) {
		json::Dict result = json::Builder{}.StartDict()
			.Key("request_id"s).Value(stop.at("id"s).AsDouble())
			.EndDict().Build().AsDict();
		const auto* stop_info = handler_.GetBusesByStop(stop.at("name"s).AsString());

		if (!stop_info) {
			return json::Builder{}.StartDict()
				.Key("request_id"s).Value(stop.at("id"s).AsDouble())
				.Key("error_message"s).Value("not found"s)
				.EndDict().Build().AsDict();
		}

		json::Array buses;
		for (const auto& bus : *stop_info) {
			buses.push_back(bus->number);
		}
		result["buses"s] = buses;

		return result;
	}

	json::Dict JsonReader::GetMapAnswer(const json::Dict& map) {
		std::stringstream strm;
		handler_.RenderMap().Render(strm);
		return json::Builder{}.StartDict()
			.Key("request_id"s).Value(map.at("id"s).AsDouble())
			.Key("map"s).Value(strm.str())
			.EndDict().Build().AsDict();
	}

	json::Dict JsonReader::GetRoutAnswer(const json::Dict& route) {
		std::string stop1 = route.at("from"s).AsString();
		std::string stop2 = route.at("to"s).AsString();
		const auto routers = transport_router_.GetRoute(stop1, stop2);
		if (!routers) {
			return json::Builder{}.StartDict()
				.Key("request_id"s).Value(route.at("id"s).AsDouble())
				.Key("error_message"s).Value("not found"s)
				.EndDict().Build().AsDict();
		}

		json::Array items;
		for (const auto& info : routers->info) {
			items.push_back(json::Builder{}.StartDict()
				.Key("type"s).Value("Wait"s)
				.Key("stop_name"s).Value(info.stop_name->name)
				.Key("time"s).Value(info.bus_wait_time)
				.EndDict().Build().AsDict());
			items.push_back(json::Builder{}.StartDict()
				.Key("type"s).Value("Bus"s)
				.Key("bus"s).Value(info.bus_name->number)
				.Key("span_count"s).Value(info.span_count)
				.Key("time"s).Value(info.time)
				.EndDict().Build().AsDict());
		}

		return json::Builder{}.StartDict()
			.Key("request_id"s).Value(route.at("id"s).AsDouble())
			.Key("total_time"s).Value(routers->total_time)
			.Key("items"s).Value(items)
			.EndDict().Build().AsDict();
	}

	void JsonReader::OutQuery(std::ostream& output) {
		json::Array result;
		const json::Array& output_document = documents_.GetRoot().AsDict().at("stat_requests"s).AsArray();
		for (const auto& doc : output_document) {
			json::Dict answer;

			const json::Dict& doc_map = doc.AsDict();
			if (doc_map.at("type"s).AsString() == "Bus"s) {
				answer = std::move(GetBusAnswer(doc_map));
			}
			else if (doc_map.at("type"s).AsString() == "Stop"s) {
				answer = std::move(GetStopAnswer(doc_map));
			}
			else if (doc_map.at("type"s).AsString() == "Route"s) {
				answer = std::move(GetRoutAnswer(doc_map));
			}
			else {
				answer = GetMapAnswer(doc_map);
			}
			result.push_back(answer);
		}
		json::Print(json::Document{ result }, output);
	}

	svg::Color JsonReader::GetColor(const json::Node& node_color) const {
		svg::Color color;
		if (node_color.IsString()) {
			color = node_color.AsString();
		}
		else {
			const json::Array clr_arr = node_color.AsArray();
			if (clr_arr.size() == 3) {
				color = svg::Rgb{ static_cast<uint8_t>(clr_arr[0].AsInt()), static_cast<uint8_t>(clr_arr[1].AsInt()), static_cast<uint8_t>(clr_arr[2].AsInt()) };
			}
			else {
				color = svg::Rgba{ static_cast<uint8_t>(clr_arr[0].AsInt()), static_cast<uint8_t>(clr_arr[1].AsInt()), static_cast<uint8_t>(clr_arr[2].AsInt()), clr_arr[3].AsDouble() };
			}
		}

		return color;
	}

	renderer::MapSetting JsonReader::GetSetting() const {
		renderer::MapSetting settings;

		const json::Dict& rndr_doc = documents_.GetRoot().AsDict().at("render_settings"s).AsDict();
		settings.width = rndr_doc.at("width"s).AsDouble();
		settings.height = rndr_doc.at("height"s).AsDouble();
		settings.padding = rndr_doc.at("padding"s).AsDouble();
		settings.line_width = rndr_doc.at("line_width"s).AsDouble();
		settings.stop_radius = rndr_doc.at("stop_radius"s).AsDouble();
		settings.bus_label_font_size = static_cast<uint32_t>(rndr_doc.at("bus_label_font_size"s).AsInt());

		const json::Array blo_arr = rndr_doc.at("bus_label_offset"s).AsArray();
		settings.bus_label_offset = { blo_arr[0].AsDouble(), blo_arr[1].AsDouble() };
		settings.stop_label_font_size = static_cast<uint32_t>(rndr_doc.at("stop_label_font_size"s).AsInt());

		const json::Array slo_arr = rndr_doc.at("stop_label_offset"s).AsArray();
		settings.stop_label_offset = { slo_arr[0].AsDouble(), slo_arr[1].AsDouble() };
		settings.underlayer_color = GetColor(rndr_doc.at("underlayer_color"s));
		settings.underlayer_width = rndr_doc.at("underlayer_width"s).AsDouble();

		std::vector<svg::Color> color_palette;
		for (const auto& node_color : rndr_doc.at("color_palette"s).AsArray()) {
			color_palette.push_back(GetColor(node_color));
		}
		settings.color_palette = std::move(color_palette);

		return settings;
	}

	void JsonReader::GetMap() {
		renderer_.AddSetting(GetSetting());
		renderer_.RenderCoords(catalogue_.GetBuses());
	}

	void JsonReader::OutMap(std::ostream& output) {
		handler_.RenderMap().Render(output);
	}

	void JsonReader::Serialize() {
		serialization::Serializer serializer(catalogue_, renderer_, transport_router_);
		std::string filename = documents_.GetRoot().AsDict().at("serialization_settings"s).AsDict().at("file"s).AsString();
		serializer.Serialize(filename);
	}

	void JsonReader::Deserialize() {
		serialization::Serializer serializer(catalogue_, renderer_, transport_router_);
		std::string filename = documents_.GetRoot().AsDict().at("serialization_settings"s).AsDict().at("file"s).AsString();
		serializer.Deserialize(filename);
	}

}