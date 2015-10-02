#pragma once
#include "static.h"

BEGIN_NAMESPACE(Core)

struct MutationInfo
{
	static std::shared_ptr<MutationInfo> compare(const Document& prev, const Document& cur) noexcept;

	enum class ChangeType { Added, Removed, Mutated };

	struct Index
	{
		size_t position;
		NodePtr parent;

		explicit Index(size_t position, NodePtr parent = nullptr)
			: position(position)
			, parent(parent)
		{
		}

		friend bool operator==(const Index& lhs, const Index& rhs)
		{
			return lhs.position == rhs.position
				&& lhs.parent == rhs.parent;
		}

		friend bool operator!=(const Index& lhs, const Index& rhs)
		{
			return !(lhs == rhs);
		}
	};

	template <typename T>
	struct Change
	{
		T prev, cur;
		ChangeType type;
		Index index;

		Change(const T& prev, const T& cur, ChangeType type, const Index& index)
			: prev(prev)
			, cur(cur)
			, type(type)
			, index(index)
		{
		}

		friend bool operator==(const Change& lhs, const Change& rhs)
		{
			return lhs.prev == rhs.prev
				&& lhs.cur == rhs.cur
				&& lhs.type == rhs.type
				&& lhs.index == rhs.index;
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