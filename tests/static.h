#pragma once

#include <bandit/bandit.h>
#include <tree/tree_util.h>

#include <core/factory.h>
#include <core/metadata.h>
#include <core/node.h>
#include <core/project.h>
#include <core/mutation_info.h>
#include <core/utils.h>

namespace snowhouse
{
	template<>
	struct Stringizer<Core::NodePtr>
	{
		static std::string ToString(const Core::NodePtr& n)
		{
			if (!n) return "nullptr";
			return "Node(" + Core::prop<std::string>(n, "$Title", 0) + ")";
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
