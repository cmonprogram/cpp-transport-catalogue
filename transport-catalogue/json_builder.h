#include "json.h"
#include <vector>
#include <deque>
#include <optional>
#include <memory>
#include <list>
#include <map>

namespace json {



	class Builder {
	public:
		using KeyType = std::string;
		using ValueType = std::variant<std::nullptr_t, Array, Dict, bool, int, double, std::string>;

		class BaseContext;
		class ArrayItemContext;
		class DictItemContext;
		class KeyContext;

	private:
		struct NodeListElem {
			std::optional<KeyType> key = std::nullopt;
			//std::optional<ValueType> value = std::nullopt;

			std::optional<Node> node = std::nullopt;
			NodeListElem* prev = nullptr;
			NodeListElem* next = nullptr;
			std::vector<NodeListElem*> arrys;

			~NodeListElem() {
				for (NodeListElem* elem : arrys) {
					delete elem;
				}
			}

			void MakeNextNode(const ValueType& input) {
				next = new NodeListElem();
				next->prev = this;
				next->node = Node(input);
				arrys.push_back(next);
				//return list_elem;
			}

			NodeListElem& operator=(const NodeListElem& input) {
				key = input.key;
				node = input.node;
				prev = input.prev;
				return *this;
			}

			void Clear() {
				key = std::nullopt;
				//value = std::nullopt;
			}
			void SetKey(const KeyType& input) {
				key = input;
			}

			void SetValue(const ValueType& input) {
				//*value = input;
				if (node.has_value() && node.value().IsArray()) {
					node.value().AsArray().push_back(input);
				}
				else if (node.has_value() && node.value().IsDict()) {
					auto test = node.value().AsDict();
					node.value().AsDict().insert({ *key, input });
					Clear();
				}
				else {
					node = Node(input);
				}
			}

			bool Empty() {
				return !node.has_value();
			}
		};
		NodeListElem* root;
		NodeListElem* current;
		enum class Commands
		{
			Key,
			Value,
			StartDict,
			EndDict,
			StartArray,
			EndArray,
			Build
		};

		std::vector<Commands> command_log;

		std::map<Commands, int> CommandsStat() {
			std::map<Commands, int> commands;
			for (Commands command : command_log) {
				++commands[command];
			}
			return commands;
		}

		bool ArrayDicCheck() {
			std::map<Commands, int> commands = CommandsStat();
			if (command_log.size()) {
				if (commands[Commands::StartArray] != commands[Commands::EndArray]) return false;
				if (commands[Commands::StartDict] != commands[Commands::EndDict]) return false;
			}
			return true;
		}

		bool ObjIsReady() {
			bool t1 = !root->Empty();
			bool t2 = current == root;
			bool t3 = !current->key.has_value();
			bool t4 = true;
			if (command_log.size()) {
				t4 = ArrayDicCheck();
			}
			else {
				t4 = false;
			}

			return t1 && t2 && t3 && t4;
		}

		bool CheckValStart() {
			//Вызов Value, StartDict или StartArray где-либо, кроме как после конструктора, после Key или после предыдущего элемента массив
			bool t1 = command_log.size() == 0;
			bool t2 = command_log.size() > 0 && command_log.at(command_log.size() - 1) == Commands::Key;
			bool t3 = command_log.size() > 0 && current->node.has_value() && current->node.value().IsArray();
			return t1 || t2 || t3;
		}

		bool CheckKey() {
			//std::logic_error при вызове метода Key снаружи словаря или сразу после другого Key.
			bool t1 = command_log.size() != 0;
			bool t2 = current->node.value().IsDict();
			bool t3 = command_log.size() > 0 && command_log.at(command_log.size() - 1) != Commands::Key;
			return t1 && t2 && t3;
		}

		void OnCommand(Commands command) {
			if (command == Commands::Key) {
				if (!CheckKey()) {
					throw std::logic_error("Wrong key");
				}
				if (ObjIsReady()) {
					throw std::logic_error("Wrong root");
				}
			}
			else if (command == Commands::Value) {
				if (!CheckValStart()) {
					throw std::logic_error("Wrong Value");
				}

				if (ObjIsReady()) {
					throw std::logic_error("Wrong root");
				}
			}
			else if (command == Commands::StartDict) {
				if (!CheckValStart()) {
					throw std::logic_error("Wrong StartDict");
				}

				if (ObjIsReady()) {
					throw std::logic_error("Wrong root");
				}
			}
			else if (command == Commands::EndDict) {
				if (ObjIsReady()) {
					throw std::logic_error("Wrong root");
				}
				if (!current->node.value().IsDict()) {
					throw std::logic_error("Wrong EndDict");
				}
			}
			else if (command == Commands::StartArray) {
				if (!CheckValStart()) {
					throw std::logic_error("Wrong StartDict");
				}

				if (ObjIsReady()) {
					throw std::logic_error("Wrong root");
				}
			}
			else if (command == Commands::EndArray) {
				if (ObjIsReady()) {
					throw std::logic_error("Wrong root");
				}
				if (!current->node.value().IsArray()) {
					throw std::logic_error("Wrong EndArray");
				}
			}
			else if (command == Commands::Build) {
				if (!ObjIsReady()) {
					throw std::logic_error("Wrong root");
				}
			}
			command_log.push_back(command);
		}
	public:
		Builder() {
			root = new NodeListElem();
			current = root;
		}
		~Builder() {
			delete root;
		}


		Builder& Value(const std::variant<std::nullptr_t, Array, Dict, bool, int, double, std::string> input) {
			OnCommand(Commands::Value);
			current->SetValue(input);
			return *this;
		}
		KeyContext Key(const std::string& input) {
			OnCommand(Commands::Key);
			current->SetKey(input);
			return KeyContext(*this);
		}


		DictItemContext StartDict() {
			OnCommand(Commands::StartDict);
			if (root->Empty()) {
				root->SetValue(Dict());
			}
			else {
				current->MakeNextNode(Dict());
				auto ptr = current->next;
				current = ptr;
			}
			return DictItemContext(*this);
		}

		ArrayItemContext StartArray() {
			OnCommand(Commands::StartArray);
			if (root->Empty()) {
				root->SetValue(Array());
			}
			else {
				current->MakeNextNode(Array());
				auto ptr = current->next;
				current = ptr;
			}
			return ArrayItemContext(*this);
		}

		Builder& EndDict() {
			OnCommand(Commands::EndDict);
			if (current->prev) {
				current->prev->SetValue(current->node.value().AsDict());
				current = current->prev;
			}
			return *this;
		}


		Builder& EndArray() {
			OnCommand(Commands::EndArray);
			if (current->prev) {
				current->prev->SetValue(current->node.value().AsArray());
				current = current->prev;
			}
			return *this;
		}

		Document Build() {
			OnCommand(Commands::Build);
			Document doc(root->node.value());
			return doc;
		}



		class BaseContext {
		public:
			BaseContext(Builder& builder) : builder_(builder) {}
			Builder& builder_;

			Builder& EndArray() {
				builder_.EndArray();
				return builder_;
			};
			Builder& EndDict() {
				builder_.EndDict();
				return builder_;
			};
			KeyContext Key(std::string input) {
				builder_.Key(input);
				return KeyContext(builder_);
			};
			ArrayItemContext StartArray() {
				builder_.StartArray();
				return ArrayItemContext(builder_);
			};
			DictItemContext StartDict() {
				builder_.StartDict();
				return DictItemContext(builder_);
			};
			Builder& Value(json::Builder::ValueType input) {
				builder_.Value(input);
				return builder_;
			};
			Builder& Build() {
				builder_.Build();
				return builder_;
			};
		};

		class ValueValueContext : public BaseContext
		{
		public:
			//После вызова StartArray и серии Value следует не Value, не StartDict, не StartArray и не EndArray.
			ValueValueContext(Builder& builder) : BaseContext(builder) {};
			//Builder& Value(json::Builder::ValueType input) = delete;
			Builder& Key(std::string input) = delete;
			//Builder& StartArray() = delete;
			//Builder& EndArray() = delete;
			//Builder& StartDict() = delete;
			Builder& EndDict() = delete;
			Builder& Build() = delete;
		};

		class ArrayValueContext : public BaseContext
		{
		public:
			ArrayValueContext(Builder& builder) : BaseContext(builder) {};
			ValueValueContext Value(json::Builder::ValueType input) {
				builder_.Value(input);
				return ValueValueContext(builder_);
			};
			Builder& Key(std::string input) = delete;
			//Builder& StartArray() = delete;
			//Builder& EndArray() = delete;
			//Builder& StartDict() = delete;
			Builder& EndDict() = delete;
			Builder& Build() = delete;
		};

		class ArrayItemContext : public BaseContext
		{
		public:
			ArrayItemContext(Builder& builder) : BaseContext(builder) {};
			ArrayValueContext Value(json::Builder::ValueType input) {
				builder_.Value(input);
				return ArrayValueContext(builder_);
			};
			Builder& Key(std::string input) = delete;
			//Builder& StartArray() = delete;
			//Builder& EndArray() = delete;
			//Builder& StartDict() = delete;
			Builder& EndDict() = delete;
			Builder& Build() = delete;
		};

		class DictItemContext : public BaseContext
		{
		public:
			//DictItemContext удалены следующие методы: Build, Value, EndArray, StartDict и StartArray.
			DictItemContext(Builder& builder) : BaseContext(builder) {};
			Builder& Value(json::Builder::ValueType input) = delete;
			//Builder& Key(std::string input) = delete;
			Builder& StartArray() = delete;
			Builder& EndArray() = delete;
			Builder& StartDict() = delete;
			//Builder& EndDict() = delete;
			Builder& Build() = delete;
		};

		class ValueKeyContext : public BaseContext
		{
		public:
			//Непосредственно после Key вызван не Value, не StartDict и не StartArray.
			ValueKeyContext(Builder& builder) : BaseContext(builder) {};
			Builder& Value(json::Builder::ValueType input) = delete;
			//Builder& Key(std::string input) = delete;
			Builder& StartArray() = delete;
			Builder& EndArray() = delete;
			Builder& StartDict() = delete;
			//Builder& EndDict() = delete;
			Builder& Build() = delete;
		};



		class KeyContext : public BaseContext
		{
		public:
			//Непосредственно после Key вызван не Value, не StartDict и не StartArray.
			KeyContext(Builder& builder) : BaseContext(builder) {};
			ValueKeyContext Value(json::Builder::ValueType input) {
				builder_.Value(input);
				return ValueKeyContext(builder_);
			};
			Builder& Key(std::string input) = delete;
			//Builder& StartArray() = delete;
			Builder& EndArray() = delete;
			//Builder& StartDict() = delete;
			Builder& EndDict() = delete;
			Builder& Build() = delete;
		};



	};
}

