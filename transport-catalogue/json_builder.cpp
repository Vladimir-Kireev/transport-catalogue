#include "json_builder.h"

namespace json {
using namespace std::literals;

Builder::Builder() {
}

Builder::KeyItemContext Builder::Key(std::string str) {
	if (nodes_stack_.empty() || !nodes_stack_.back()->IsDict() || key_.has_value()) {
		throw std::logic_error("Method \"Key\" cannot be called."s);
	}
	key_ = str;

	return KeyItemContext(*this);
}

bool Builder::GetNewRoot(Node::Value value, bool is_value) {
	Node result = std::visit([](auto& args) -> Node {return args; }, value);
	if (is_start_build_) {
		is_start_build_ = false;
		root_ = std::move(result);
		if (!is_value) {
			nodes_stack_.emplace_back(&root_);
		}
		return true;
	}
	if (key_.has_value()) {
		Dict& dict = const_cast<Dict&>(nodes_stack_.back()->AsDict());
		dict.emplace(*key_, std::move(result));
		if (!is_value) {
			nodes_stack_.emplace_back(&dict[*key_]);
		}
		key_.reset();
		return true;
	}
	if (!nodes_stack_.empty() && nodes_stack_.back()->IsArray()) {
		Array& array = const_cast<Array&>(nodes_stack_.back()->AsArray());
		array.push_back(std::move(result));
		if (!is_value) {
			nodes_stack_.emplace_back(&array.back());
		}
		return true;
	}
	return false;
}

Builder::ItemContext Builder::Value(Node::Value value) {
	if (GetNewRoot(std::move(value), true)) {
		return ItemContext(*this);
	}
	throw std::logic_error("Method \"Value\" cannot be called."s);
}

Builder::DictItemContext Builder::StartDict() {
	Dict result{};
	if (GetNewRoot(result, false)) {
		return DictItemContext(*this);
	}
	throw std::logic_error("Method \"StartDict\" cannot be called."s);
}

Builder::ArrayItemContext Builder::StartArray() {
	Array result{};
	if (GetNewRoot(result, false)) {
		return ArrayItemContext(*this);
	}
	throw std::logic_error("Method \"StartArray\" cannot be called."s);
}

Builder::ItemContext Builder::EndDict() {
	size_t stack_size = nodes_stack_.size();
	if (stack_size == 1 && nodes_stack_[0]->IsDict()) {
		nodes_stack_.clear();
		return ItemContext(*this);
	}
	if (stack_size > 1) {
		if (nodes_stack_[stack_size - 2]->IsDict() || nodes_stack_[stack_size - 2]->IsArray()) {
			nodes_stack_.pop_back();
			return ItemContext(*this);;
		}
	}
	throw std::logic_error("Method \"EndDict\" cannot be called. The building Dict is not completed."s);
}

Builder::ItemContext Builder::EndArray() {
	size_t stack_size = nodes_stack_.size();
	if (stack_size == 1 && nodes_stack_[0]->IsArray()) {
		nodes_stack_.clear();
		return ItemContext(*this);
	}
	if (stack_size > 1) {
		if (nodes_stack_[stack_size - 2]->IsDict() || nodes_stack_[stack_size - 2]->IsArray()) {
			nodes_stack_.pop_back();
			return ItemContext(*this);
		}
	}
	throw std::logic_error("Method \"EndArray\" cannot be called. The building Array is not completed."s);
}

json::Node Builder::Build() {
	if (is_start_build_ || nodes_stack_.size() > 0) {
		throw std::logic_error("Method \"Build\" cannot be called. The building is not completed."s);
	}
	return root_;
}


Builder::ItemContext::ItemContext(Builder& builder) : builder_(builder) {
}

Builder::KeyItemContext Builder::ItemContext::Key(std::string str) {
	return builder_.Key(std::move(str));
}
Builder::ItemContext Builder::ItemContext::Value(Node::Value value) {
	return builder_.Value(std::move(value));
}
Builder::DictItemContext Builder::ItemContext::StartDict() {
	return builder_.StartDict();
}
Builder::ArrayItemContext Builder::ItemContext::StartArray() {
	return builder_.StartArray();
}
Builder::ItemContext Builder::ItemContext::EndDict() {
	return builder_.EndDict();
}
Builder::ItemContext Builder::ItemContext::EndArray() {
	return builder_.EndArray();
}
json::Node Builder::ItemContext::Build() {
	return builder_.Build();
}


Builder::DictItemContext::DictItemContext(Builder& builder) : ItemContext(builder) {
}

Builder::KeyItemContext Builder::DictItemContext::Key(std::string str) {
	return ItemContext::Key(std::move(str));
}

Builder::ItemContext Builder::DictItemContext::EndDict() {
	return ItemContext::EndDict();
}


Builder::ArrayItemContext::ArrayItemContext(Builder& builder) : ItemContext(builder) {
}

Builder::ArrayItemContext Builder::ArrayItemContext::Value(Node::Value value) {
	ItemContext::Value(std::move(value));
	return *this;
}

Builder::DictItemContext Builder::ArrayItemContext::StartDict() {
	return ItemContext::StartDict();;
}

Builder::ArrayItemContext Builder::ArrayItemContext::StartArray() {
	return ItemContext::StartArray();
}

Builder::ItemContext Builder::ArrayItemContext::EndArray() {
	return ItemContext::EndArray();
}


Builder::KeyItemContext::KeyItemContext(Builder& builder) : ItemContext(builder) {
}

Builder::DictItemContext Builder::KeyItemContext::Value(Node::Value value) {
	ItemContext::Value(std::move(value));
	return DictItemContext(builder_);
}

Builder::DictItemContext Builder::KeyItemContext::StartDict() {
	return ItemContext::StartDict();
}

Builder::ArrayItemContext Builder::KeyItemContext::StartArray() {
	return ItemContext::StartArray();;
}
} //namespace