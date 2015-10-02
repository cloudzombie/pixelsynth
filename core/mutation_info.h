#pragma once
#include "static.h"

BEGIN_NAMESPACE(Core)

struct MutationInfo
{
	static std::shared_ptr<MutationInfo> compare(const Document& prev, const Document& cur) noexcept;

	enum class ChangeType { Added, Removed, Mutated };

	template <typename T>
	struct Change
	{
		T prev;
		T cur;
		ChangeType type;
		NodePtr parent;

		Change(const T prev, const T cur, ChangeType type, const NodePtr parent)
			: prev(prev)
			, cur(cur)
			, type(type)
			, parent(parent)
		{
		}

		friend bool operator==(const Change& lhs, const Change& rhs)
		{
			return lhs.prev == rhs.prev
				&& lhs.cur == rhs.cur
				&& lhs.type == rhs.type
				&& lhs.parent == rhs.parent;
		}

		friend bool operator!=(const Change& lhs, const Change& rhs)
		{
			return !(lhs == rhs);
		}
	};

	template <typename T>
	using ChangeSet = std::vector<Change<T>>;

	ChangeSet<NodePtr> nodes;
	ChangeSet<PropertyPtr> properties;
	ChangeSet<ConnectorMetadataPtr> connectors;
	ChangeSet<ConnectionPtr> connections;
};

END_NAMESPACE(Core)