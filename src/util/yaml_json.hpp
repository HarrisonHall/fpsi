#pragma once

#include <iostream>
#include <string>

#include "json.hpp"

#if __has_include("yaml.h")
#include "yaml.h"
#elif __has_include("yaml-cpp/yaml.h")
#include <yaml-cpp/yaml.h>
#else
#error "yaml-cpp has not been built locally or is not installed on system"
#endif


namespace util {

// Adapted from tojson https://github.com/mircodezorzi/tojson
inline nlohmann::ordered_json parse_scalar(const YAML::Node &node) {
	int i;
	double d;
	bool b;
	std::string s;
	
	if (YAML::convert<int>::decode(node, i)) return i;
	if (YAML::convert<double>::decode(node, d)) return d;
	if (YAML::convert<bool>::decode(node, b)) return b;
	if (YAML::convert<std::string>::decode(node, s)) return s;
	
	return nullptr;
}

// Adapted from tojson https://github.com/mircodezorzi/tojson
inline nlohmann::ordered_json yaml2json(const YAML::Node &root) {
	nlohmann::ordered_json j{};
	
	switch (root.Type()) {
		case YAML::NodeType::Null: break;
		case YAML::NodeType::Scalar: return parse_scalar(root);
		case YAML::NodeType::Sequence:
			for (auto &&node : root)
				j.emplace_back(yaml2json(node));
			break;
		case YAML::NodeType::Map:
			for (auto &&it : root)
				j[it.first.as<std::string>()] = yaml2json(it.second);
			break;
		default: break;
	}
	return j;
}

nlohmann::ordered_json from_yaml(const YAML::Node &orig) {
	return yaml2json(orig);
}

}
