#include "svg.h"

namespace svg {

std::ostream& operator<<(std::ostream& out, const StrokeLineCap line_cap) {
    switch (line_cap)
    {
    case StrokeLineCap::BUTT:
        out << "butt"sv;
        break;
    case StrokeLineCap::ROUND:
        out << "round"sv;
        break;
    case StrokeLineCap::SQUARE:
        out << "square"sv;
        break;
    default:
        break;
    }
    return out;
}

std::ostream& operator<<(std::ostream& out, const StrokeLineJoin line_join) {
    switch (line_join)
    {
    case StrokeLineJoin::ARCS:
        out << "arcs"sv;
        break;
    case StrokeLineJoin::BEVEL:
        out << "bevel"sv;
        break;
    case StrokeLineJoin::MITER:
        out << "miter"sv;
        break;
    case StrokeLineJoin::MITER_CLIP:
        out << "miter-clip"sv;
        break;
    case StrokeLineJoin::ROUND:
        out << "round"sv;
        break;
    default:
        break;
    }
    return out;
}

std::ostream& operator<<(std::ostream& out, const Color color) {
    std::visit(OstreamColorPrinter{ out }, color);
    return out;
}

void Object::Render(const RenderContext& context) const {
    context.RenderIndent();

    // Делегируем вывод тега своим подклассам
    RenderObject(context);

    context.out << std::endl;
}

// ---------- Circle ------------------

Circle& Circle::SetCenter(Point center)  {
    center_ = center;
    return *this;
}

Circle& Circle::SetRadius(double radius)  {
    radius_ = radius;
    return *this;
}

void Circle::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
    out << "r=\""sv << radius_ << "\" "sv;
    RenderAttrs(context.out);
    out << "/>"sv;
}

// ---------- Polyline ----------------

Polyline& Polyline::AddPoint(Point point) {
    points_.push_back(point);
    return *this;
}

void Polyline::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<polyline points=\"";
    if (points_.empty()) {
        out << "\"";
    }
    else {
        for (size_t i = 0; i < points_.size() - 1; ++i) {
            out << points_[i].x << ","sv << points_[i].y << " "sv;
        }
        out << points_.back().x << ","sv << points_.back().y << "\""sv;
    }
    RenderAttrs(context.out);
    out << " />"sv;
}

// ---------- Text ----------------

Text& Text::SetPosition(Point pos) {
    position_ = pos;
    return *this;
}

Text& Text::SetOffset(Point offset) {
    offset_ = offset;
    return *this;
}

Text& Text::SetFontSize(uint32_t size) {
    font_size_ = size;
    return *this;
}

Text& Text::SetFontFamily(std::string font_family) {
    font_family_ = font_family;
    return *this;
}

Text& Text::SetFontWeight(std::string font_weight) {
    font_weight_ = font_weight;
    return *this;
}

Text& Text::SetData(std::string data) {
    data_ = data;
    ReplaceAll(data_);
    return *this;
}

void Text::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<text"sv;
    RenderAttrs(context.out);
    out << " x = \""sv << position_.x << "\" y=\""sv << position_.y << "\" dx=\""sv << offset_.x << "\" dy=\""sv << offset_.y << "\" font-size=\""sv << font_size_;
    if (!font_family_.empty()) {
        out << "\" font-family=\""sv << font_family_;
    }
    if (!font_weight_.empty()) {
        out << "\" font-weight=\""sv << font_weight_;
    }
    out << "\">"sv << data_ << "</text>"sv;
}

void Text::ReplaceAll(std::string& str) {
    std::vector<std::pair<std::string_view, std::string_view>> sep = {
        {"&"sv, "&amp;"sv},
        {"\""sv, "&quot;"sv},
        {"\'"sv, "&apos;"sv},
        {"<"sv, "&lt;"sv},
        {">"sv, "&gt;"sv} };

    for (const auto& [key, value] : sep) {
        for (std::string::size_type pos{};
            (pos = str.find(key.data(), pos, key.length())) != str.npos;
            pos += value.length()) {
            str.replace(pos, key.length(), value.data(), value.length());
        }
    }
}

// ---------- Document ----------------

    // Добавляет в svg-документ объект-наследник svg::Object
void Document::AddPtr(std::unique_ptr<Object>&& obj) {
    objects_.push_back(std::move(obj));
}

// Выводит в ostream svg-представление документа
void Document::Render(std::ostream& out) const {
    out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"sv << std::endl;
    out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"sv << std::endl;
    RenderContext ctx(out, 2, 2);
    for (const auto& obj : objects_) {
        obj->Render(ctx);
    }
    out << "</svg>"sv;
}

}  // namespace svg