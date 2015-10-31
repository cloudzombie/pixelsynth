#include "mutation_info.h"
#include "document.h"

using Core::ConnectionPtr;
using Core::ConnectorMetadataCollection;
using Core::ConnectorMetadataPtr;
using Core::Document;
using Core::MutationInfo;
using Core::Node;
using Core::NodePtr;
using Core::PropertyPtr;
using Core::node_eq_uuid;

using ChangeType = MutationInfo::ChangeType;
template <typename T>
using Change = MutationInfo::Change<T>;
template <typename T>
using ChangeSet = MutationInfo::ChangeSet<T>;

struct Index
{
	const NodePtr prevParent;
	size_t prevIndex;

	const NodePtr curParent;
	size_t curIndex;
};

/*
	When determing what has mutated there can be different types of data to check:
	* A NodeNodePair, because a NodePtr may have changed into a new NodePtr whenever its properties or connectors has changed
	* A tuple that indicates which property or connection (with a NodePtr as a parent) has changed, along with a size_t to indicate its position in the list of properties / connectors
	* Connections themselves
*/

using NodeNodePair = std::pair<NodePtr, NodePtr>;
using NodePropertyTuple = std::tuple<NodePtr, PropertyPtr, size_t>;
using NodeConnectorTuple = std::tuple<NodePtr, ConnectorMetadataPtr, size_t>;

enum TupleIndex
{
	Node = 0,
	Data = 1,
	Position = 2
};

namespace std
{
	template<>
	struct hash<NodeNodePair>
	{
		typedef NodeNodePair argument_type;
		typedef size_t result_type;

		result_type operator()(argument_type const& pair) const
		{
			result_type h {};
			if (pair.first) h ^= hash<NodePtr>()(pair.first);
			if (pair.second) h ^= hash<NodePtr>()(pair.second);
			return h;
		}
	};

	template<>
	struct hash<NodePropertyTuple>
	{
		typedef NodePropertyTuple argument_type;
		typedef size_t result_type;

		result_type operator()(argument_type const& tuple) const
		{
			auto h = hash<NodePtr>()(get<TupleIndex::Node>(tuple));
			h ^= get<TupleIndex::Data>(tuple)->metadata().hash();
			return h;
		}
	};

	template<>
	struct hash<NodeConnectorTuple>
	{
		typedef NodeConnectorTuple argument_type;
		typedef size_t result_type;

		result_type operator()(argument_type const& tuple) const
		{
			auto h = hash<NodePtr>()(get<TupleIndex::Node>(tuple));
			h ^= get<TupleIndex::Data>(tuple)->hash();
			return h;
		}
	};
}

// Takes Input and inserts it as Data into the ChangeSet
template <typename Input, typename Data>
struct Inserter
{
	ChangeSet<Data> data;

	void operator()(Input prev, Input cur, ChangeType type, Index index)
	{
		data.insert(Change<Data>(prev, cur, type, index.prevParent, index.curParent, index.prevIndex, index.curIndex));
	}
};

// Extracts the data from a tuple and inserts it
template <typename Data>
struct Inserter<std::tuple<NodePtr, Data, size_t>, Data>
{
	ChangeSet<Data> data;

	void operator()(std::tuple<NodePtr, Data, size_t> prev, std::tuple<NodePtr, Data, size_t> cur, ChangeType type, Index index)
	{
		data.insert(Change<Data>(std::get<TupleIndex::Data>(prev), std::get<TupleIndex::Data>(cur), type, index.prevParent, index.curParent, index.prevIndex, index.curIndex));
	}
};

// A dummy index provider
struct NullIndexProvider
{
	template <typename Container, typename Iterator>
	Index operator()(const Document& prevDoc, const Container& prevContainer, Iterator& prevIterator, const Document& curDoc, const Container& curContainer, Iterator& curIterator)
	{
		return { nullptr, static_cast<size_t>(-1), nullptr, static_cast<size_t>(-1) };
	}
};

// Extracts the index from the tuple
struct TupleIndexProvider
{
	template <typename Container, typename Iterator>
	Index operator()(const Document& prevDoc, const Container& prevContainer, Iterator& prevIterator, const Document& curDoc, const Container& curContainer, Iterator& curIterator)
	{
		return {
			prevIterator == cend(prevContainer) ? nullptr : std::get<TupleIndex::Node>(*prevIterator), prevIterator == cend(prevContainer) ? -1 : static_cast<size_t>(std::distance(cbegin(prevContainer), prevIterator)),
			curIterator == cend(curContainer) ? nullptr : std::get<TupleIndex::Node>(*curIterator), curIterator == cend(curContainer) ? -1 : static_cast<size_t>(std::distance(cbegin(curContainer), curIterator))
		};
	}
};

// Queries the tree for the index based on the parent
struct TreeParentIndexProvider
{
	template <typename Container, typename Iterator>
	Index operator()(const Document& prevDoc, const Container& prevContainer, Iterator& prevIterator, const Document& curDoc, const Container& curContainer, Iterator& curIterator)
	{
		return {
			prevIterator == cend(prevContainer) ? nullptr : prevDoc.parent(**prevIterator), prevIterator == cend(prevContainer) ? -1 : prevDoc.childIndex(**prevIterator),
			curIterator == cend(curContainer) ? nullptr : curDoc.parent(**curIterator), curIterator == cend(curContainer) ? -1 : curDoc.childIndex(**curIterator)
		};
	}
};

template <typename IndexProvider, typename EqualityFn, typename Inserter, typename Container>
void compareItems(Inserter& inserter, const Document& prevDocument, const Document& curDocument, const Container& prev, const Container& cur)
{
	auto curIt = cbegin(cur);
	while (curIt != cend(cur))
	{
		auto prevIt = prev.find(*curIt);
		if (prevIt == cend(prev))
		{
			// added
			inserter({}, *curIt, ChangeType::Added, IndexProvider()(prevDocument, prev, prevIt, curDocument, cur, curIt));
		}
		else
		{
			if (!EqualityFn(prevDocument, curDocument, *curIt)(*prevIt))
			{
				// mutated
				inserter(*prevIt, *curIt, ChangeType::Mutated, IndexProvider()(prevDocument, prev, prevIt, curDocument, cur, curIt));
			}
		}

		++curIt;
	}

	auto prevIt = cbegin(prev);
	while (prevIt != cend(prev))
	{
		curIt = cur.find(*prevIt);
		if (curIt == cend(cur))
		{
			// removed
			inserter(*prevIt, {}, ChangeType::Removed, IndexProvider()(prevDocument, prev, prevIt, curDocument, cur, curIt));
		}

		++prevIt;
	}
}

template <typename Result, typename Tuple, typename EqualityFn, typename SetKeyFn, typename GetContainerFn>
auto compareAffectedNodes(const Document& prev, const Document& cur, std::unordered_set<NodeNodePair> affectedNodes, GetContainerFn getContainer)
{
	Inserter<Tuple, Result> propertyInserter;

	// For every node, do processing and add the results to the inserter
	for (auto&& affectedNode : affectedNodes)
	{
		// Copy the props/connections to a set so we can use .find on them, based on uuid or hash
		// Also make them into tuples so we can store the parent and the position in the parent's list
		std::set<Tuple, SetKeyFn> prevItems;
		std::set<Tuple, SetKeyFn> curItems;
		if (affectedNode.first)
		{
			size_t index = 0;
			for (auto&& p : getContainer(affectedNode.first)) prevItems.insert(make_tuple(affectedNode.first, p, index++));
		}
		if (affectedNode.second)
		{
			size_t index = 0;
			for (auto&& p : getContainer(affectedNode.second)) curItems.insert(make_tuple(affectedNode.second, p, index++));
		}
		compareItems<TupleIndexProvider, EqualityFn>(propertyInserter, prev, cur, prevItems, curItems);
	}

	return propertyInserter.data;
}

struct node_eq_ptr_and_parent
{
	explicit node_eq_ptr_and_parent(const Document& prev, const Document& cur, const NodePtr self):prev_(prev),cur_(cur),self_(self) { }
	bool operator()(NodePtr other) const { return other == self_ && prev_.parent(*self_) == cur_.parent(*other); }
private:
	const Document& prev_;
	const Document& cur_;
	const NodePtr self_;
};

struct connection_eq_ptr
{
	explicit connection_eq_ptr(const Document& prev, const Document& cur, const ConnectionPtr self):self_(self) { }
	bool operator()(ConnectionPtr other) const { return other == self_; }
private:
	const ConnectionPtr self_;
};

struct node_uuid_key_fn
{
	bool operator() (const NodePtr& x, const NodePtr& y) const { return x->uuid() == y->uuid(); }
};

struct propertytuple_eq_ptr
{
	explicit propertytuple_eq_ptr(const Document& prev, const Document& cur, NodePropertyTuple self): self(self) { }
	bool operator()(NodePropertyTuple other) const { return std::get<TupleIndex::Data>(self) == std::get<TupleIndex::Data>(other); }
private:
	NodePropertyTuple self;
};

struct connectortuple_eq_ptr
{
	explicit connectortuple_eq_ptr(const Document& prev, const Document& cur, NodeConnectorTuple self): self(self) { }
	bool operator()(NodeConnectorTuple other) const { return std::get<TupleIndex::Data>(self) == std::get<TupleIndex::Data>(other); }
private:
	NodeConnectorTuple self;
};

struct propertytuple_uuid_less_fn
{
	bool operator() (const NodePropertyTuple& x, const NodePropertyTuple& y) const
	{
		// compare property's index in node
		return std::get<TupleIndex::Position>(x) < std::get<TupleIndex::Position>(y);
	}
};

struct connectortuple_uuid_less_fn
{
	bool operator() (const NodeConnectorTuple& x, const NodeConnectorTuple& y) const
	{
		// compare connector's index in node
		return std::get<TupleIndex::Position>(x) < std::get<TupleIndex::Position>(y);
	}
};

template <typename EqualityFn, typename DestContainer, typename SrcContainer>
void copyDiffering(const Document& prev, const Document& cur, DestContainer& prevDest, DestContainer& curDest, SrcContainer& prevSrc, SrcContainer& curSrc)
{
	copy(begin(curSrc), end(curSrc), inserter(curDest, begin(curDest)));
	copy_if(begin(prevSrc), end(prevSrc), inserter(prevDest, begin(prevDest)), [&prev, &cur, &curDest](const auto& prevItem)
	{
		auto curIt = find_if(cbegin(curDest), cend(curDest), EqualityFn(prev, cur, prevItem));
		auto isDifferent = curIt == cend(curDest);
		if (!isDifferent) curDest.erase(curIt);
		return isDifferent;
	});
}

std::shared_ptr<MutationInfo> MutationInfo::compare(const Document& prev, const Document& cur) noexcept
{
	auto info = std::make_shared<MutationInfo>(prev, cur);

	// Create two sets that contain all prev/cur nodes and make them comparable and findable by uuid
	// To limit the amount of processing we need to do, we remove nodes from cur that are exactly the same in prev, and also don't copy those nodes to prev
	std::unordered_set<NodePtr, std::hash<NodePtr>, node_uuid_key_fn> prevNodes, curNodes;
	copyDiffering<node_eq_ptr_and_parent>(prev, cur, prevNodes, curNodes, prev.nodes(), cur.nodes());

	// Remove the root nodes, they should never be considered changed
	prevNodes.erase(prev.root());
	curNodes.erase(cur.root());

	//
	Inserter<NodePtr, NodePtr> nodeInserter;
	compareItems<TreeParentIndexProvider, node_eq_ptr_and_parent>(nodeInserter, prev, cur, prevNodes, curNodes);
	info->nodes = nodeInserter.data;

	// To limit the amount of processing when checking props/connectors, only look at nodes that have mutated
	std::unordered_set<NodeNodePair> affectedNodes;
	for (auto&& n : info->nodes)
	{
		if (n.prev || n.cur) affectedNodes.insert({ n.prev, n.cur });
	}
	info->properties = compareAffectedNodes<PropertyPtr, NodePropertyTuple, propertytuple_eq_ptr, propertytuple_uuid_less_fn>(prev, cur, affectedNodes, [](const NodePtr& node) { return node->properties(); });
	info->connectors = compareAffectedNodes<ConnectorMetadataPtr, NodeConnectorTuple, connectortuple_eq_ptr, connectortuple_uuid_less_fn>(prev, cur, affectedNodes, [](const NodePtr& node) { return node->connectorMetadata(); });

	// Create two sets that contain all prev/cur connections except for the ones that are absolutely identical
	std::unordered_set<ConnectionPtr> prevConnections, curConnections;
	copyDiffering<connection_eq_ptr>(prev, cur, prevConnections, curConnections, prev.connections(), cur.connections());

	// Compare the connections
	Inserter<ConnectionPtr, ConnectionPtr> connectionInserter;
	compareItems<NullIndexProvider, connection_eq_ptr>(connectionInserter, prev, cur, prevConnections, curConnections);
	info->connections = connectionInserter.data;

	return info;
}
