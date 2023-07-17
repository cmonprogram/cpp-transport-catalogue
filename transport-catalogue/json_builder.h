#pragma once
#include "json.h"
#include <vector>
#include <deque>
#include <optional>
#include <memory>
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

			~NodeListElem();

			void MakeNextNode(const ValueType& input);

			void Clear();
			void SetKey(const KeyType& input);
			void SetValue(const ValueType& input);
			bool Empty();
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
		std::map<Commands, int> CommandsStat();
		bool ArrayDicCheck();
		bool ObjIsReady();
		bool CheckValStart();

		bool CheckKey() {
			//std::logic_error при вызове метода Key снаружи словаря или сразу после другого Key.
			bool t1 = command_log.size() != 0;
			bool t2 = current->node.value().IsDict();
			bool t3 = command_log.size() > 0 && command_log.at(command_log.size() - 1) != Commands::Key;
			return t1 && t2 && t3;
		}
		void OnCommand(Commands command);
	public:
		Builder();
		~Builder();


		Builder& Value(const std::variant<std::nullptr_t, Array, Dict, bool, int, double, std::string> input);
		KeyContext Key(const std::string& input);
		DictItemContext StartDict();
		ArrayItemContext StartArray();
		Builder& EndDict();
		Builder& EndArray();
		Document Build();


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

		class ArrayItemContext : public BaseContext
		{
		public:
			//4,5 За вызовом StartArray следует не Value, не StartDict, не StartArray и не EndArray.
			ArrayItemContext(Builder& builder) : BaseContext(builder) {};
			ArrayItemContext Value(json::Builder::ValueType input) {
				builder_.Value(input);
				return ArrayItemContext(builder_);
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
			//3. За вызовом StartDict следует не Key и не EndDict.
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
			//2. После вызова Value, последовавшего за вызовом Key, вызван не Key и не EndDict.
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
			//1. Непосредственно после Key вызван не Value, не StartDict и не StartArray.
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