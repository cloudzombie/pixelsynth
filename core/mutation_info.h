#pragma once
#include "static.h"

BEGIN_NAMESPACE(Core)

struct MutationInfo
{
	MutationInfo(const Document& prev, const Document& cur);

	enum class ChangeType { Added, Removed, Mutated };

	template <typename T>
	struct Change
	{
		T prev;
		T cur;
		ChangeType type;
		NodePtr prevParent;
		NodePtr curParent;
		size_t prevIndex;
		size_t curIndex;

		Change(const T prev, const T cur, ChangeType type, const NodePtr prevParent, const NodePtr curParent, size_t prevIndex, size_t curIndex)
			: prev(prev)
			, cur(cur)
			, type(type)
			, prevParent(prevParent)
			, curParent(curParent)
			, prevIndex(prevIndex)
			, curIndex(curIndex)
		{
		}

		friend bool operator==(const Change& lhs, const Change& rhs)
		{
			return lhs.prev == rhs.prev
				&& lhs.cur == rhs.cur
				&& lhs.type == rhs.type
				&& lhs.prevParent == rhs.prevParent
				&& lhs.curParent == rhs.curParent
				&& lhs.prevIndex == rhs.prevIndex
				&& lhs.curIndex == rhs.curIndex;
		}

		friend bool operator!=(const Change& lhs, const Change& rhs)
		{
			return !(lhs == rhs);
		}

		friend std::ostream& operator<<(std::ostream& out, const Change<T>& c)
		{
			std::string type;
			switch (c.type)
			{
			case ChangeType::Added:
				type = "Added";
				break;
			case ChangeType::Removed:
				type = "Removed";
				break;
			case ChangeType::Mutated:
				type = "Mutated";
				break;
			}

			out << "Change(" << type << ": " << *c.prev << ", " << *c.cur << ", " << *c.prevParent << ", " << *c.curParent << ", index: " << c.prevIndex << ", " << c.curIndex << ")";
			return out;
		}
		friend std::ostream& operator<<(std::ostream& out, Change<T>* c) { out << *c; return out; }
	};

	template <typename T>
	using ChangeSet = std::vector<Change<T>>;

	ChangeSet<NodePtr> nodes;
	ChangeSet<PropertyPtr> properties;
	ChangeSet<ConnectorMetadataPtr> connectors;
	ChangeSet<ConnectionPtr> connections;

	const Document& prev;
	const Document& cur;

	std::vector<NodePtr> prevNodes;
	std::vector<NodePtr> curNodes;
};

END_NAMESPACE(Core)