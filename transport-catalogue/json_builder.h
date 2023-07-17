#pragma once

#include "json.h"

#include <vector>
#include <string>
#include <optional>

namespace json {

class Builder {
public:
	class KeyItemContext;
	class ItemContext;
	class DictItemContext;
	class ArrayItemContext;

	class ItemContext {
	public:
		ItemContext(Builder& builder);

		KeyItemContext Key(std::string str);
		ItemContext Value(Node::Value value);
		DictItemContext StartDict();
		ArrayItemContext StartArray();
		ItemContext EndDict();
		ItemContext EndArray();
		json::Node Build();

	protected:
		Builder& builder_;
	};

	class DictItemContext final : public ItemContext {
	public:
		DictItemContext(Builder& builder);

		KeyItemContext Key(std::string str);
		ItemContext EndDict();

		ItemContext Value(Node::Value value) = delete;
		DictItemContext StartDict() = delete;
		ArrayItemContext StartArray() = delete;
		ItemContext EndArray() = delete;
		json::Node Build() = delete;
	};

	class ArrayItemContext final : public ItemContext {
	public:
		ArrayItemContext(Builder& builder);

		ArrayItemContext Value(Node::Value value);
		DictItemContext StartDict();
		ArrayItemContext StartArray();
		ItemContext EndArray();

		KeyItemContext Key(std::string str) = delete;
		ItemContext EndDict() = delete;
		json::Node Build() = delete;
	};

	class KeyItemContext final : public ItemContext {
	public:
		KeyItemContext(Builder& builder);

		DictItemContext Value(Node::Value value);
		DictItemContext StartDict();
		ArrayItemContext StartArray();

		KeyItemContext Key(std::string str) = delete;
		ItemContext EndDict() = delete;
		ItemContext EndArray() = delete;
		json::Node Build() = delete;
	};

	Builder();

	KeyItemContext Key(std::string str);
	ItemContext Value(Node::Value value);
	DictItemContext StartDict();
	ArrayItemContext StartArray();
	ItemContext EndDict();
	ItemContext EndArray();
	json::Node Build();

private:
	bool GetNewRoot(Node::Value value, bool is_value);

	Node root_;
	std::vector<Node*> nodes_stack_;
	std::optional<std::string> key_;
	bool is_start_build_ = true;
};
} //namespace