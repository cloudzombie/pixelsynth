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

		friend bool operator<(const Change& lhs, const Change& rhs)
		{
			if (lhs.type == rhs.type)
			{
				switch (lhs.type)
				{
				case ChangeType::Added:
					return (lhs.curIndex < rhs.curIndex)
						|| (lhs.curIndex == rhs.curIndex && lhs.curParent < rhs.curParent);
				case ChangeType::Removed:
					// When removing we want to iterate based on previous indices
					return (lhs.prevIndex < rhs.prevIndex)
						|| (lhs.prevIndex == rhs.prevIndex && lhs.prevParent < rhs.prevParent);
				case ChangeType::Mutated:
				default:
					return (lhs.prevIndex < rhs.prevIndex)
						|| (lhs.prevIndex == rhs.prevIndex && lhs.curIndex < rhs.curIndex)
						|| (lhs.prevIndex == rhs.prevIndex && lhs.curIndex == rhs.curIndex && lhs.prevParent < rhs.prevParent)
						|| (lhs.prevIndex == rhs.prevIndex && lhs.curIndex == rhs.curIndex == lhs.prevParent < rhs.prevParent && lhs.curParent < rhs.curParent);
				}
			}
			else
				return lhs.type < rhs.type;
		}

		friend bool operator<=(const Change& lhs, const Change& rhs)
		{
			return !(rhs < lhs);
		}

		friend bool operator>(const Change& lhs, const Change& rhs)
		{
			return rhs < lhs;
		}

		friend bool operator>=(const Change& lhs, const Change& rhs)
		{
			return !(lhs < rhs);
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
	using ChangeSet = std::set<Change<T>>;

	ChangeSet<NodePtr> nodes;
	ChangeSet<PropertyPtr> properties;
	ChangeSet<ConnectorMetadataPtr> connectors;
	ChangeSet<ConnectionPtr> connections;

	const Document& prev;
	const Document& cur;

	MutationInfo(const Document& prev, const Document& cur)
		: prev(prev),
		  cur(cur)
	{
	}
};

END_NAMESPACE(Core)