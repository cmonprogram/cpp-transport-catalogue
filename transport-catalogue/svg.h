#pragma once

#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <optional>
#include <variant>
#include <sstream>
#include <cstdint>
namespace svg {

	struct Rgb {
		Rgb() = default;
		int red = 0;
		int green = 0;
		int blue = 0;

		Rgb(const int& red, const int& green, const int& blue)
			: red(red), green(green), blue(blue)
		{
		}
	};

	struct Rgba {
		Rgba() = default;
		int red = 0;
		int green = 0;
		int blue = 0;
		double opacity = 1.0;

		Rgba(const int& red, const int& green, const int& blue, double opacity)
			: red(red), green(green), blue(blue), opacity(opacity)
		{
			if (opacity > 1 || opacity < 0) throw std::out_of_range("");
		}
	};


	using Color = std::variant<std::monostate, std::string, Rgb, Rgba>;



	struct ColorPrinter {
		std::string operator()(std::monostate) {
			return "none";
		}
		std::string operator()(std::string inp) {
			return inp;
		}
		std::string operator()(Rgb inp) {
			std::stringstream stream;
			stream << "rgb(" << std::to_string(inp.red) << "," << std::to_string(inp.green) << "," << std::to_string(inp.blue) << ")";
			return stream.str();
		}
		std::string operator()(Rgba inp) {
			std::stringstream stream;
			stream << "rgba(" << std::to_string(inp.red) << "," << std::to_string(inp.green) << "," << std::to_string(inp.blue) << "," << inp.opacity << ")";
			return stream.str();
		}
	};
	inline const Color NoneColor = std::monostate();

	std::ostream& operator<<(std::ostream& out, const Color& inp);

	enum class StrokeLineCap {
		BUTT,
		ROUND,
		SQUARE,
	};

	enum class StrokeLineJoin {
		ARCS,
		BEVEL,
		MITER,
		MITER_CLIP,
		ROUND,
	};
	std::ostream& operator<<(std::ostream& out, const StrokeLineCap& inp);

	std::ostream& operator<<(std::ostream& out, const StrokeLineJoin& inp);


	template <typename Owner>
	class PathProps {
	public:
		Owner& SetFillColor(Color color) {
			fill_color_ = std::move(color);
			return AsOwner();
		}
		Owner& SetStrokeColor(Color color) {
			stroke_color_ = std::move(color);
			return AsOwner();
		}
		Owner& SetStrokeWidth(double width) {
			stroke_width_ = width;
			return AsOwner();
		}
		Owner& SetStrokeLineCap(StrokeLineCap line_cap) {
			line_cap_ = line_cap;
			return AsOwner();
		}
		Owner& SetStrokeLineJoin(StrokeLineJoin line_join) {
			line_join_ = line_join;
			return AsOwner();
		}
	protected:
		~PathProps() = default;

		void RenderAttrs(std::ostream& out) const {
			using namespace std::literals;
			if (fill_color_) {
				out << " fill=\""sv << *fill_color_ << "\""sv;
			}
			if (stroke_color_) {
				out << " stroke=\""sv << *stroke_color_ << "\""sv;
			}
			if (stroke_width_) {
				out << " stroke-width=\""sv << *stroke_width_ << "\""sv;
			}
			if (line_cap_) {
				out << " stroke-linecap=\""sv << *line_cap_ << "\""sv;
			}
			if (line_join_) {
				out << " stroke-linejoin=\""sv << *line_join_ << "\""sv;
			}

		}
	private:
		Owner& AsOwner() {
			return static_cast<Owner&>(*this);
		}
		std::optional<Color> fill_color_;
		std::optional<Color> stroke_color_;
		std::optional<double> stroke_width_;
		std::optional<StrokeLineCap> line_cap_;
		std::optional<StrokeLineJoin> line_join_;
	};




	struct Point {
		Point() = default;
		Point(double x, double y)
			: x(x)
			, y(y) {
		}
		double x = 0;
		double y = 0;
	};




	/*
	 * Вспомогательная структура, хранящая контекст для вывода SVG-документа с отступами.
	 * Хранит ссылку на поток вывода, текущее значение и шаг отступа при выводе элемента
	 */
	struct RenderContext {
		RenderContext(std::ostream& out)
			: out(out) {
		}

		RenderContext(std::ostream& out, int indent_step, int indent = 0)
			: out(out)
			, indent_step(indent_step)
			, indent(indent) {
		}

		RenderContext Indented() const {
			return { out, indent_step, indent + indent_step };
		}

		void RenderIndent() const {
			for (int i = 0; i < indent; ++i) {
				out.put(' ');
			}
		}

		std::ostream& out;
		int indent_step = 0;
		int indent = 0;
	};

	/*
	 * Абстрактный базовый класс Object служит для унифицированного хранения
	 * конкретных тегов SVG-документа
	 * Реализует паттерн "Шаблонный метод" для вывода содержимого тега
	 */
	class Object {
	public:
		void Render(const RenderContext& context) const;

		virtual ~Object() = default;

	private:
		virtual void RenderObject(const RenderContext& context) const = 0;
	};


	class ObjectContainer {
	public:
		template <typename Obj>
		void Add(Obj obj) {
			figures_.emplace_back(std::make_unique<Obj>(std::move(obj)));
		}

		// Добавляет в svg-документ объект-наследник svg::Object
		virtual void AddPtr(std::unique_ptr<Object>&& obj) = 0;
	protected:
		~ObjectContainer() = default;
		//std::vector<Object> figures;
		std::vector<std::unique_ptr<Object>> figures_;
	};

	class Drawable {
	public:
		virtual ~Drawable() = default;
		virtual void Draw(ObjectContainer& container) const = 0;
	};

	/*
	 * Класс Circle моделирует элемент <circle> для отображения круга
	 * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/circle
	 */
	class Circle final : public Object, public PathProps<Circle> {
	public:
		Circle& SetCenter(Point center);
		Circle& SetRadius(double radius);

	private:
		void RenderObject(const RenderContext& context) const override;

		Point center_;
		double radius_ = 1.0;
	};

	/*
	 * Класс Polyline моделирует элемент <polyline> для отображения ломаных линий
	 * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/polyline
	 */
	class Polyline : public Object, public PathProps<Polyline> {
	public:
		// Добавляет очередную вершину к ломаной линии
		Polyline& AddPoint(Point point);

		/*
		 * Прочие методы и данные, необходимые для реализации элемента <polyline>
		 */
	private:
		void RenderObject(const RenderContext& context) const override;
		std::vector<Point> points_;
	};

	/*
	 * Класс Text моделирует элемент <text> для отображения текста
	 * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/text
	 */
	class Text : public Object, public PathProps<Text> {
	public:
		// Задаёт координаты опорной точки (атрибуты x и y)
		Text& SetPosition(Point pos);

		// Задаёт смещение относительно опорной точки (атрибуты dx, dy)
		Text& SetOffset(Point offset);

		// Задаёт размеры шрифта (атрибут font-size)
		Text& SetFontSize(uint32_t size);

		// Задаёт название шрифта (атрибут font-family)
		Text& SetFontFamily(std::string font_family);

		// Задаёт толщину шрифта (атрибут font-weight)
		Text& SetFontWeight(std::string font_weight);

		// Задаёт текстовое содержимое объекта (отображается внутри тега text)
		Text& SetData(std::string data);

		std::string escape(const std::string& input) const;
		// Прочие данные и методы, необходимые для реализации элемента <text>
	private:
		void RenderObject(const RenderContext& context) const override;
		std::optional<Point> pos_ = Point{ 0.0, 0.0 };
		std::optional <Point> offset_ = Point{ 0.0, 0.0 };
		std::optional <uint32_t> size_ = 1;
		std::optional <std::string> font_family_ = std::nullopt;
		std::optional <std::string> font_weight_ = std::nullopt;
		std::optional <std::string> data_ = std::nullopt;
	};

	class Document : public ObjectContainer {
	public:
		// Добавляет в svg-документ объект-наследник svg::Object
		void AddPtr(std::unique_ptr<Object>&& obj);

		// Выводит в ostream svg-представление документа
		void Render(std::ostream& out) const;

	};

}  // namespace svg