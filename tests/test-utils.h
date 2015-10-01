#pragma once

#include "static.h"

using namespace Core;

static std::shared_ptr<Node> makeNode(HashValue type, std::string title)
{
	auto builder = Factory::makeNode(type);
	builder->mutateProperty(hash("Title"), [&](auto& prop) { prop.set(0, title); });
	return std::make_shared<Node>(std::move(*builder));
}
