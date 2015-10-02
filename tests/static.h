#pragma once

#include <bandit/bandit.h>
#include <tree/tree_util.h>

#include <core/factory.h>
#include <core/metadata.h>
#include <core/node.h>
#include <core/project.h>
#include <core/mutation_info.h>

inline Core::PropertyPtr prop(const Core::NodePtr& node, const char* propertyTitle)
{
	auto prop = find_if(begin(node->properties()), end(node->properties()), Core::property_eq_hash(node->nodeType(), Core::hash(propertyTitle)));
	if (prop == end(node->properties())) return nullptr;
	return *prop;
}

inline size_t propIndex(const Core::NodePtr& node, const char* propertyTitle)
{
	auto prop = find_if(begin(node->properties()), end(node->properties()), Core::property_eq_hash(node->nodeType(), Core::hash(propertyTitle)));
	return distance(begin(node->properties()), prop);
}

template <typename T>
inline T prop(const Core::NodePtr& node, const char* propertyTitle, Core::Frame frame)
{
	auto p = prop(node, propertyTitle);
	if (!p) return T();
	return p->get<T>(frame);
}

inline std::shared_ptr<const Core::Node> findNode(Core::Project& project, std::string nodeTitle)
{
	auto result = std::find_if(cbegin(project.current().nodes()), cend(project.current().nodes()), [nodeTitle](auto& node)
	{
		return prop<std::string>(node, "Title", 0) == nodeTitle;
	});
	if (result == cend(project.current().nodes())) return std::shared_ptr<const Core::Node>();
	return *result;
}

inline Core::ConnectorMetadataPtr connector(std::shared_ptr<const Core::Node> node, std::string connectorTitle)
{
	auto result = find_if(cbegin(node->connectorMetadata()), cend(node->connectorMetadata()), [connectorTitle](auto& metadata)
	{
		return metadata->hash() == Core::hash(connectorTitle.c_str());
	});
	if (result == cend(node->connectorMetadata())) return nullptr;
	return *result;
}

inline size_t connectorIndex(std::shared_ptr<const Core::Node> node, std::string connectorTitle)
{
	auto result = find_if(cbegin(node->connectorMetadata()), cend(node->connectorMetadata()), [connectorTitle](auto& metadata)
	{
		return metadata->hash() == Core::hash(connectorTitle.c_str());
	});
	return distance(cbegin(node->connectorMetadata()), result);
}

namespace snowhouse
{
	template<>
	struct Stringizer<Core::NodePtr>
	{
		static std::string ToString(const Core::NodePtr& n)
		{
			if (!n) return "nullptr";
			return "Node(" + prop<std::string>(n, "Title", 0) + ")";
		}
	};

	template<>
	struct Stringizer<Core::PropertyPtr>
	{
		static std::string ToString(const Core::PropertyPtr& p)
		{
			return "Property(" + p->metadata().title() + ")";
		}
	};

	template<typename T>
	struct Stringizer<Core::MutationInfo::Change<T>>
	{
		static std::string ToString(const Core::MutationInfo::Change<T>& c)
		{
			std::string type;
			switch (c.type)
			{
			case Core::MutationInfo::ChangeType::Added:
				type = "Added";
			case Core::MutationInfo::ChangeType::Removed:
				type = "Removed";
			case Core::MutationInfo::ChangeType::Mutated:
				type = "Mutated";
			}
			return "Change(" 
				+ Stringizer<T>::ToString(c.prev) 
				+ ", " 
				+ Stringizer<T>::ToString(c.cur) 
				+ ", " 
				+ type 
				+ ", " 
				+ Stringizer<Core::NodePtr>::ToString(c.parent)
				+ ")";
		}
	};
}
