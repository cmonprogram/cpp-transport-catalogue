#pragma once

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <variant>
#include <sstream>

namespace json {

	class Node;

	// Сохраните объявления Dict и Array без изменения
	using Dict = std::map<std::string, Node>;
	using Array = std::vector<Node>;

	// Эта ошибка должна выбрасываться при ошибках парсинга JSON
	class ParsingError : public std::runtime_error {
	public:
		using runtime_error::runtime_error;
	};

	class Node : public std::variant<std::nullptr_t, Array, Dict, int, double, std::string, bool> {

	public:
		/* Реализуйте Node, используя std::variant */
		using variant::variant;
		using Value = std::variant<std::nullptr_t, Array, Dict, int, double, std::string, bool>;
		Node() = default;


		template<typename Type>
		const Type& As() const {
			try {
				return std::get<Type>(*this);
			}
			catch (const std::bad_variant_access&) {
				throw std::logic_error("");
			}
		}

		template<typename Type>
		bool Is() const {
			return std::get_if<Type>(this);
		}

		bool IsInt() const;
		bool IsDouble() const;// Возвращает true, если в Node хранится int либо double.
		bool IsPureDouble() const;// Возвращает true, если в Node хранится double.
		bool IsBool() const;
		bool IsString() const;
		bool IsNull() const;
		bool IsArray() const;
		bool IsMap() const;

		bool AsBool() const;
		double AsDouble() const;
		const Array& AsArray() const;
		const Dict& AsMap() const;
		int AsInt() const;
		const std::string& AsString() const;

		struct PrintContext {
			std::ostream& out;
			int indent_step = 4;
			int indent = 0;

			void PrintIndent() const {
				for (int i = 0; i < indent; ++i) {
					out.put(' ');
				}
			}

			// Возвращает новый контекст вывода с увеличенным смещением
			PrintContext Indented() const {
				return { out, indent_step, indent_step + indent };
			}
		};

		void PrintValue(const nullptr_t&, PrintContext& con) const {
			con.out << "null";
		}
		void PrintValue(Array inp, PrintContext& con) const {
			con.out << "[";
			if (inp.size() > 0) {
				inp.at(0).PrintNode(con.out);
				bool first = true;
				for (const auto& val : inp) {
					if (first) {
						first = false;
					}
					else {
						con.out << ",";
						val.PrintNode(con.out);
					}

				}
			}
			con.out << "]";
		}
		void PrintValue(Dict inp, PrintContext& con) const {
			con.out << "{";
			if (inp.size() > 0) {
				con.out << "\"";
				con.out << inp.begin()->first;
				con.out << "\"";
				con.out << ":";
				inp.begin()->second.PrintNode(con.out);

				bool first = true;
				for (const auto& [key, val] : inp) {
					if (first) {
						first = false;
					}
					else {
						con.out << ",";
						con.out << "\"";
						con.out << key;
						con.out << "\"";
						con.out << ":";
						val.PrintNode(con.out);
					}

				}
			}
			con.out << "}";

		}
		void PrintValue(bool inp, PrintContext& con) const {
			inp ? con.out << "true" : con.out << "false";
		}
		void PrintValue(std::string inp, PrintContext& con) const {
			std::string result;
			result.push_back('"');
			for (const char& ch : inp) {
				if (ch == '\"') {
					result.push_back('\\');
					result.push_back('\"');
				}
				else if (ch == '\\') {
					result.push_back('\\');
					result.push_back('\\');
				}
				else if (ch == '\r') {
					result.push_back('\\');
					result.push_back('r');
				}
				else if (ch == '\n') {
					result.push_back('\\');
					result.push_back('n');
				}
				else {
					result.push_back(ch);
				}
			}
			result.push_back('"');
			con.out << result;
		}
		template <typename Type>
		void PrintValue(Type val, PrintContext& con) const {
			con.out << val;
		}

		void PrintNode(/*NodeValue& val, */std::ostream& out) const {
			auto val = static_cast<Value>(*this);
			PrintContext context{ out };
			std::visit([&](auto& val) {
				PrintValue(val, context);
				}, val);
		};

		bool operator==(const Node& inp) const {
			if (this->index() == inp.index() && !this->valueless_by_exception()) {
				return static_cast<Value>(*this) == static_cast<Value>(inp);
			}
			return false;
		}
		bool operator!=(const Node& inp) const {
			return !(*this == inp);
		}



	};

	class Document {
	public:
		bool operator==(const Document& inp) {
			return this->GetRoot() == inp.GetRoot();
		}
		bool operator!=(const Document& inp) {
			return !(*this == inp);
		}
		explicit Document(Node root);

		const Node& GetRoot() const;

	private:
		Node root_;
	};

	Document Load(std::istream& input);

	void Print(const Document& doc, std::ostream& output);

}  // namespace json