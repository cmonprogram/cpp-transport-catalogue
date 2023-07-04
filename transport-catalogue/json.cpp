#include "json.h"

namespace json {


	namespace {
		Node LoadNode(std::istream& input);

		Node LoadArray(std::istream& input) {
			Array result;
			char c;
			for (; input >> c && c != ']';) {
				if (c != ',') {
					input.putback(c);
				}
				result.push_back(LoadNode(input));
			}
			if (c != ']') throw ParsingError("] expected");
			return Node(std::move(result));
		}

		// Считывает содержимое строкового литерала JSON-документа
		// Функцию следует использовать после считывания открывающего символа ":
		std::string LoadString(std::istream& input) {
			using namespace std::literals;

			auto it = std::istreambuf_iterator<char>(input);
			auto end = std::istreambuf_iterator<char>();
			//std::cout << *it;
			std::string s;
			while (true) {
				if (it == end) {
					// Поток закончился до того, как встретили закрывающую кавычку?
					throw ParsingError("String parsing error");
				}
				const char ch = *it;
				if (ch == '"') {
					// Встретили закрывающую кавычку
					++it;
					break;
				}
				else if (ch == '\\') {
					// Встретили начало escape-последовательности
					++it;
					if (it == end) {
						// Поток завершился сразу после символа обратной косой черты
						throw ParsingError("String parsing error");
					}
					const char escaped_char = *(it);
					// Обрабатываем одну из последовательностей: \\, \n, \t, \r, \"
					switch (escaped_char) {
					case 'n':
						s.push_back('\n');
						break;
					case 't':
						s.push_back('\t');
						break;
					case 'r':
						s.push_back('\r');
						break;
					case '"':
						s.push_back('"');
						break;
					case '\\':
						s.push_back('\\');
						break;
					default:
						// Встретили неизвестную escape-последовательность
						throw ParsingError("Unrecognized escape sequence \\"s + escaped_char);
					}
				}
				else if (ch == '\n' || ch == '\r') {
					// Строковый литерал внутри- JSON не может прерываться символами \r или \n
					throw ParsingError("Unexpected end of line"s);
				}
				else {
					// Просто считываем очередной символ и помещаем его в результирующую строку
					s.push_back(ch);
				}
				++it;
			}

			return s;
		}
		Node LoadDict(std::istream& input) {
			Dict result;
			char c;
			for (; input >> c && c != '}';) {
				if (c == ',') {
					input >> c;
				}

				std::string key = LoadString(input);
				input >> c;
				result.insert({ std::move(key), LoadNode(input) });
			}
			if (c != '}') throw ParsingError("} expected");
			return Node(std::move(result));
		}

		using Number = std::variant<int, double>;

		Number LoadNumber(std::istream& input) {
			using namespace std::literals;

			std::string parsed_num;

			// Считывает в parsed_num очередной символ из input
			auto read_char = [&parsed_num, &input] {
				parsed_num += static_cast<char>(input.get());
				if (!input) {
					throw ParsingError("Failed to read number from stream"s);
				}
			};

			// Считывает одну или более цифр в parsed_num из input
			auto read_digits = [&input, read_char] {
				if (!std::isdigit(input.peek())) {
					//std::string test;
					//input >> test;
					throw ParsingError("A digit is expected"s);
				}
				while (std::isdigit(input.peek())) {
					read_char();
				}
			};

			if (input.peek() == '-') {
				read_char();
			}
			// Парсим целую часть числа
			if (input.peek() == '0') {
				read_char();
				// После 0 в JSON не могут идти другие цифры
			}
			else {
				read_digits();
			}

			bool is_int = true;
			// Парсим дробную часть числа
			if (input.peek() == '.') {
				read_char();
				read_digits();
				is_int = false;
			}

			// Парсим экспоненциальную часть числа
			if (int ch = input.peek(); ch == 'e' || ch == 'E') {
				read_char();
				if (ch = input.peek(); ch == '+' || ch == '-') {
					read_char();
				}
				read_digits();
				is_int = false;
			}

			try {
				if (is_int) {
					// Сначала пробуем преобразовать строку в int
					try {
						return std::stoi(parsed_num);
					}
					catch (...) {
						// В случае неудачи, например, при переполнении,
						// код ниже попробует преобразовать строку в double
					}
				}
				return std::stod(parsed_num);
			}
			catch (...) {
				throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
			}
		}



		Node LoadNode(std::istream& input) {
			char skip;
			while (input.get(skip)) {
				if (skip != '\t'
					&& skip != '\r'
					&& skip != '\n'
					&& skip != ' ') {
					input.putback(skip);
					break;
				}
			}
			input.clear();
			char word[6];
			input.get(word, 6);

			std::string word_str(word);

			if (word_str.compare(0, 4, "null") == 0) {
				for (int i = word_str.size() - 1; i >= 4; i--) {
					input.putback(word_str[i]);
				}
				return Node();
			}
			if (word_str.compare(0, 4, "true") == 0) {
				for (int i = word_str.size() - 1; i >= 4; i--) {
					input.putback(word_str[i]);
				}
				return Node(true);
			}
			if (word_str.compare(0, 5, "false") == 0) {
				for (int i = word_str.size() - 1; i >= 5; i--) {
					input.putback(word_str[i]);
				}
				return Node(false);
			}
			for (int i = word_str.size() - 1; i >= 0; i--) {
				input.putback(word_str[i]);
			}


			char c;
			//input >> c;
			input.get(c);
			//auto test = (c == '[');
			if (c == '[') {
				return LoadArray(input);
			}
			else if (c == '{') {
				return LoadDict(input);
			}
			else if (c == '"') {
				return LoadString(input);
			}
			else {
				input.putback(c);
				auto result = LoadNumber(input);
				if (result.index() == 0) {
					return Node(std::get<int>(result));
				}
				if (result.index() == 1) {
					return Node(std::get<double>(result));
				}
				throw std::runtime_error("");
			}
		}

	}// namespace



	bool Node::IsInt() const
	{
		return Is<int>();
	}

	bool Node::IsDouble() const
	{
		return IsInt() || Is<double>();
	}

	bool Node::IsPureDouble() const
	{
		return Is<double>();
	}

	bool Node::IsBool() const
	{
		return Is<bool>();
	}

	bool Node::IsString() const
	{
		return Is<std::string>();
	}

	bool Node::IsNull() const
	{
		return Is<nullptr_t>();
	}

	bool Node::IsArray() const
	{
		return Is<Array>();
	}

	bool Node::IsMap() const
	{
		return Is<Dict>();
	}


	bool Node::AsBool() const
	{
		return As<bool>();
	}

	double Node::AsDouble() const
	{
		try {
			return As<double>();
		}
		catch (const std::logic_error&) {
			return As<int>();
		}
	}

	const Array& Node::AsArray() const {
		return As<Array>();
	}

	const Dict& Node::AsMap() const {
		return As<Dict>();
	}

	int Node::AsInt() const {
		return As<int>();
	}

	const std::string& Node::AsString() const {
		return  As<std::string>();
		/*
		std::string result;
		result.push_back('"');
		for (const char& ch : As<string>()) {
			if (ch == '\"') {
				result.push_back('\\');
				result.push_back('\"');
			}
			else if (ch == '\\') {
				result.push_back('\\');
				result.push_back('\\');
			}
			else {
				result.push_back(ch);
			}
		}
		result.push_back('"');
		*/
		/*
		std::string inp = As<string>();
		std:istringstream stream(inp);
		auto test = get<string>(LoadNode(stream).data_);
		return get<string>(LoadNode(stream).data_);
		*/
	}

	Document::Document(Node root)
		: root_(std::move(root)) {
	}

	const Node& Document::GetRoot() const {
		return root_;
	}

	Document Load(std::istream& input) {
		return Document{ LoadNode(input) };
	}

	void Print(const Document& doc, std::ostream& output) {
		doc.GetRoot().PrintNode(output);
		// Реализуйте функцию самостоятельно
	}

}  // namespace json