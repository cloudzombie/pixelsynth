#pragma once
#include "static.h"

BEGIN_NAMESPACE(Core)

struct MutationInfo
{
	static std::shared_ptr<MutationInfo> compare(const Document& prev, const Document& cur) noexcept;

	template <typename T>
	struct MutationBase
	{
		std::unordered_set<T> added; // new ptr
		std::unordered_set<T> removed; // old ptr
		std::unordered_map<T, T> mutated; // old ptr --> new ptr
	};

	using NodeMutationInfo = MutationBase<NodePtr>;
	using PropertyMutationInfo = MutationBase<PropertyPtr>;
	using ConnectorMetadataMutationInfo = MutationBase<ConnectorMetadataPtr>;
	using ConnectionMutationInfo = MutationBase<ConnectionPtr>;

	std::unordered_map<NodePtr, NodeMutationInfo> nodes;
	std::unordered_map<NodePtr, PropertyMutationInfo> properties;
	std::unordered_map<NodePtr, ConnectorMetadataMutationInfo> connectorMetadata;
	ConnectionMutationInfo connections;

	using reparenting_t = std::unordered_map<NodePtr, std::pair<NodePtr, NodePtr>>; // old parent, new parent
	reparenting_t reparenting;
};

END_NAMESPACE(Core)