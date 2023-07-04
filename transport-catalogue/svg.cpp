#include "svg.h"
#include <map>
namespace svg {

	using namespace std::literals;

	void Object::Render(const RenderContext& context) const {
		context.RenderIndent();

		// Делегируем вывод тега своим подклассам
		RenderObject(context);

		context.out << std::endl;
	}



	// ---------- Circle ------------------

	Circle& Circle::SetCenter(Point center) {
		center_ = center;
		return *this;
	}

	Circle& Circle::SetRadius(double radius) {
		radius_ = radius;
		return *this;
	}

	void Circle::RenderObject(const RenderContext& context) const {
		auto& out = context.out;
		out << "<circle";
		out << " cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\""sv;
		out << " r=\""sv << radius_ << "\""sv;
		RenderAttrs(out);
		out << "/>"sv;
	}

	Polyline& Polyline::AddPoint(Point point)
	{
		points_.push_back(point);
		return *this;
	}

	void Polyline::RenderObject(const RenderContext& context) const
	{
		auto& out = context.out;
		out << "<polyline";
		out << " points=\"";
		if (points_.size()) {
			out << points_.at(0).x << "," << points_.at(0).y;
			bool first = true;
			for (const auto& point : points_) {
				if (first) {
					first = false;
				}
				else { out << " " << point.x << "," << point.y; }
			}
		}
		out << "\"";
		RenderAttrs(out);
		out << "/>";

	}

	Text& Text::SetPosition(Point pos)
	{
		pos_ = pos;
		return *this;
	}

	Text& Text::SetOffset(Point offset)
	{
		offset_ = offset;
		return *this;
	}

	Text& Text::SetFontSize(uint32_t size)
	{
		size_ = size;
		return *this;
	}

	Text& Text::SetFontFamily(std::string font_family)
	{
		font_family_ = font_family;
		return *this;
	}

	Text& Text::SetFontWeight(std::string font_weight)
	{
		font_weight_ = font_weight;
		return *this;
	}

	Text& Text::SetData(std::string data)
	{
		data_ = data;
		return *this;
	}

	std::string Text::escape(const std::string& input) const
	{
		std::map<char, std::string> simbols = {
			{'"', "&quot;"},
			{'\'', "&apos;"},
			{'<', "&lt;"},
			{'>', "&gt;"},
			{'&', "&amp;"}
		};
		std::string copy;
		for (char ch : input) {
			auto iter = simbols.find(ch);
			if (iter != simbols.end()) {
				copy += iter->second;
			}
			else {
				copy.push_back(ch);
			}
		}
		return copy;
	}

	void Text::RenderObject(const RenderContext& context) const
	{
		/*
		Двойная кавычка " заменяется на &quot;. Точка с запятой в представлении этого и следующих спецсимволов — обязательная часть экранирующей последовательности.
		Одинарная кавычка или апостроф ' заменяется на &apos;.
		Символы < и > заменяются на &lt; и &gt; соответственно.
		Амперсанд & заменяется на &amp;.

		Выполняя экранирование особых символов, следите за тем, чтобы ранее экранированные символы не подвергались повторному экранированию. Например, преобразовав символ < в последовательность &lt;, не нужно подвергать содержащийся в этой последовательности & повторному экранированию.
		*/

		auto& out = context.out;
		out << "<text";
		RenderAttrs(out);
		if (pos_) {
			out << " x=\"" << pos_.value().x << "\""
				<< " y=\"" << pos_.value().y << "\"";
		}
		if (offset_) {
			out << " dx=\"" << offset_.value().x << "\""
				<< " dy=\"" << offset_.value().y << "\"";
		}
		if (size_) {
			out << " font-size=\"" << size_.value() << "\"";
		}
		if (font_family_) {
			out << " font-family=\"" << font_family_.value() << "\"";
		}
		if (font_weight_) {
			out << " font-weight=\"" << font_weight_.value() << "\"";
		}
		out << ">";
		if (data_) {
			out << escape(data_.value());
		}
		out << "</text>";
	}


	void Document::AddPtr(std::unique_ptr<Object>&& obj)
	{
		figures_.push_back(std::move(obj));
	}

	void Document::Render(std::ostream& out) const
	{
		RenderContext ctx(out, 2, 2);
		out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"sv << std::endl;
		out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"sv << std::endl;
		for (const auto& figure : figures_) {
			figure->Render(ctx);
		}

		out << "</svg>"sv;
	}

	std::ostream& operator<<(std::ostream& out, const Color& inp)
	{
		out << std::visit(ColorPrinter{}, inp);
		return out;
	}

	std::ostream& operator<<(std::ostream& out, const StrokeLineCap& inp)
	{
		switch (inp) {
		case StrokeLineCap::BUTT:
			out << "butt";
			break;
		case StrokeLineCap::ROUND:
			out << "round";
			break;
		case StrokeLineCap::SQUARE:
			out << "square";
			break;
		}
		return out;
	}

	std::ostream& operator<<(std::ostream& out, const StrokeLineJoin& inp)
	{
		switch (inp) {
		case StrokeLineJoin::ARCS:
			out << "arcs";
			break;
		case StrokeLineJoin::BEVEL:
			out << "bevel";
			break;
		case StrokeLineJoin::MITER:
			out << "miter";
			break;
		case StrokeLineJoin::MITER_CLIP:
			out << "miter-clip";
			break;
		case StrokeLineJoin::ROUND:
			out << "round";
			break;
		}
		return out;
	}

}  // namespace svg