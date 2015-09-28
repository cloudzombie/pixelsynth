#pragma once

#include <bandit/bandit.h>
#include <core/project.h>
#include <core/node.h>

inline std::string title(std::shared_ptr<const Core::Node> node)
{
	return node->prop(Core::hash("Title")).get<std::string>(0);
}

inline std::shared_ptr<const Core::Node> findNode(Core::Project& project, std::string t)
{
	auto result = std::find_if(begin(project.current().nodes()), end(project.current().nodes()), [t](std::shared_ptr<const Core::Node> node)
	{
		return title(node) == t;
	});
	if (result == end(project.current().nodes())) return std::shared_ptr<const Core::Node>();
	return *result;
}