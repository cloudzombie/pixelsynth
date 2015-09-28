#pragma once

#include <functional>
#include <map>
#include <utility>
#include <memory>
#include <iostream>
#include <vector>
#include <eggs/variant.hpp>
#include <tree/tree.h>

#include "hash.h"

#define BEGIN_NAMESPACE(name) namespace name {
#define END_NAMESPACE(name) }

namespace Core
{
	using Frame = float;
	using PropertyValue = eggs::variant<int, double, std::string>;
};