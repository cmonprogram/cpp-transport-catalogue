#include "json_builder.h"

json::Builder::NodeListElem::~NodeListElem() {
	for (NodeListElem* elem : arrys) {
		delete elem;
	}
}

void json::Builder::NodeListElem::MakeNextNode(const ValueType& input) {
	next = new NodeListElem();
	next->prev = this;
	next->node = Node(input);
	arrys.push_back(next);
	//return list_elem;
}

void json::Builder::NodeListElem::Clear() {
	key = std::nullopt;
	//value = std::nullopt;
}

void json::Builder::NodeListElem::SetKey(const KeyType& input) {
	key = input;
}

void json::Builder::NodeListElem::SetValue(const ValueType& input) {
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

bool json::Builder::NodeListElem::Empty() {
	return !node.has_value();
}

std::map<json::Builder::Commands, int> json::Builder::CommandsStat() {
	std::map<Commands, int> commands;
	for (Commands command : command_log) {
		++commands[command];
	}
	return commands;
}

bool json::Builder::ArrayDicCheck() {
	std::map<Commands, int> commands = CommandsStat();
	if (command_log.size()) {
		if (commands[Commands::StartArray] != commands[Commands::EndArray]) return false;
		if (commands[Commands::StartDict] != commands[Commands::EndDict]) return false;
	}
	return true;
}

bool json::Builder::ObjIsReady() {
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

bool json::Builder::CheckValStart() {
	//Вызов Value, StartDict или StartArray где-либо, кроме как после конструктора, после Key или после предыдущего элемента массив
	bool t1 = command_log.size() == 0;
	bool t2 = command_log.size() > 0 && command_log.at(command_log.size() - 1) == Commands::Key;
	bool t3 = command_log.size() > 0 && current->node.has_value() && current->node.value().IsArray();
	return t1 || t2 || t3;
}

void json::Builder::OnCommand(Commands command) {
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

json::Builder::Builder() {
	root = new NodeListElem();
	current = root;
}

json::Builder::~Builder() {
	delete root;
}

json::Builder& json::Builder::Value(const std::variant<std::nullptr_t, Array, Dict, bool, int, double, std::string> input) {
	OnCommand(Commands::Value);
	current->SetValue(input);
	return *this;
}

json::Builder::KeyContext json::Builder::Key(const std::string& input) {
	OnCommand(Commands::Key);
	current->SetKey(input);
	return KeyContext(*this);
}

json::Builder::DictItemContext json::Builder::StartDict() {
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

json::Builder::ArrayItemContext json::Builder::StartArray() {
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

json::Builder& json::Builder::EndDict() {
	OnCommand(Commands::EndDict);
	if (current->prev) {
		current->prev->SetValue(current->node.value().AsDict());
		current = current->prev;
	}
	return *this;
}
json::Builder& json::Builder::EndArray() {
	OnCommand(Commands::EndArray);
	if (current->prev) {
		current->prev->SetValue(current->node.value().AsArray());
		current = current->prev;
	}
	return *this;
}

inline json::Document json::Builder::Build() {
	OnCommand(Commands::Build);
	Document doc(root->node.value());
	return doc;
}
