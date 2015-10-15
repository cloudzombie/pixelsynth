#include "static.h"
#include "mutationproject.h"

using namespace bandit;
using namespace Core;

template <typename T>
using Change = MutationInfo::Change<T>;
using ChangeType = MutationInfo::ChangeType;

go_bandit([]() {
	describe("mutation:", [&]()
	{
		std::unique_ptr<MutationProject> p;
		std::vector<std::shared_ptr<MutationInfo>> mutations;

		before_each([&]()
		{
			mutations.clear();

			p = std::make_unique<MutationProject>();
			p->setMutationCallback([&mutations](auto mutationInfo) { mutations.emplace_back(mutationInfo); });

			p->applyMutationsTo(9);
		});

		it("should have emitted mutations", [&]()
		{
			AssertThat(mutations.size(), Equals(10));
		});

		it("should emit added nodes", [&]()
		{
			auto mutation = mutations.at(0);
			AssertThat(mutation->nodes.size(), Equals(3));
			auto m = begin(mutation->nodes);
			AssertThat(*m++, Equals(Change<NodePtr>(nullptr, p->a0, ChangeType::Added, p->root(), 0)));
			AssertThat(*m++, Equals(Change<NodePtr>(nullptr, p->b0, ChangeType::Added, p->root(), 1)));
			AssertThat(*m++, Equals(Change<NodePtr>(nullptr, p->c0, ChangeType::Added, p->root(), 2)));
		});

		it("should emit removed nodes", [&]()
		{
			auto mutation = mutations.at(1);
			AssertThat(mutation->nodes.size(), Equals(1));
			AssertThat(mutation->nodes, Contains(Change<NodePtr>(p->b0, nullptr, ChangeType::Removed, p->root(), 1)));
		});

		it("should emit nodes re-added after undo", [&]()
		{
			auto mutation = mutations.at(2);
			AssertThat(mutation->nodes.size(), Equals(1));
			AssertThat(mutation->nodes, Contains(Change<NodePtr>(nullptr, p->b0, ChangeType::Added, p->root(), 1)));
		});

		it("should emit changed properties", [&]()
		{
			auto mutation = mutations.at(3);
			AssertThat(mutation->nodes.size(), Equals(1));
			AssertThat(mutation->nodes, Contains(Change<NodePtr>(p->a0, p->a3, ChangeType::Mutated, p->root(), 0)));

			AssertThat(mutation->properties.size(), Equals(5));
			AssertThat(mutation->properties, Contains(Change<PropertyPtr>(prop(*p->a0, "int"), prop(*p->a3, "int"), ChangeType::Mutated, p->a3, 1)));
			AssertThat(mutation->properties, Contains(Change<PropertyPtr>(prop(*p->a0, "double"), prop(*p->a3, "double"), ChangeType::Mutated, p->a3, 2)));
			AssertThat(mutation->properties, Contains(Change<PropertyPtr>(prop(*p->a0, "vec2"), prop(*p->a3, "vec2"), ChangeType::Mutated, p->a3, 3)));
			AssertThat(mutation->properties, Contains(Change<PropertyPtr>(prop(*p->a0, "vec3"), prop(*p->a3, "vec3"), ChangeType::Mutated, p->a3, 4)));
			AssertThat(mutation->properties, Contains(Change<PropertyPtr>(prop(*p->a0, "string"), prop(*p->a3, "string"), ChangeType::Mutated, p->a3, 5)));
		});

		it("should emit added connectors", [&]()
		{
			auto mutation = mutations.at(4);
			AssertThat(mutation->nodes.size(), Equals(1));
			AssertThat(mutation->nodes, Contains(Change<NodePtr>(p->a3, p->a4, ChangeType::Mutated, p->root(), 0)));

			AssertThat(mutation->connectors.size(), Equals(1));
			AssertThat(mutation->connectors, Contains(Change<ConnectorMetadataPtr>(nullptr, connector(*p->a4, "Foo"), ChangeType::Added, p->a4, 2)));
		});

		it("should emit removed connectors", [&]()
		{
			auto mutation = mutations.at(5);
			AssertThat(mutation->nodes.size(), Equals(1));
			AssertThat(mutation->nodes, Contains(Change<NodePtr>(p->a4, p->a5, ChangeType::Mutated, p->root(), 0)));

			AssertThat(mutation->connectors.size(), Equals(1));
			AssertThat(mutation->connectors, Contains(Change<ConnectorMetadataPtr>(connector(*p->a4, "Foo"), nullptr, ChangeType::Removed, p->a4, 2)));
		});

		it("should emit added connections", [&]()
		{
			auto mutation = mutations.at(6);
			AssertThat(mutation->connections.size(), Equals(1));
			AssertThat(mutation->connections.begin()->cur->connection(), Equals(make_tuple(p->a6, connector(*p->a6, "Out"), p->b6, connector(*p->b6, "In"))));
		});

		it("should emit removed connections", [&]()
		{
			auto mutation = mutations.at(7);
			AssertThat(mutation->connections.size(), Equals(1));
			AssertThat(mutation->connections.begin()->prev->connection(), Equals(make_tuple(p->a6, connector(*p->a6, "Out"), p->b6, connector(*p->b6, "In"))));
		});

		it("should emit reparenting from root to lower", [&]()
		{
			auto mutation = mutations.at(8);
			AssertThat(mutation->nodes.size(), Equals(2));
			AssertThat(mutation->nodes, Contains(Change<NodePtr>(p->a8, p->a8, ChangeType::Mutated, p->c8, 0)));
			AssertThat(mutation->nodes, Contains(Change<NodePtr>(p->b8, p->b8, ChangeType::Mutated, p->c8, 1)));
		});

		it("should emit reparenting from lower to root", [&]()
		{
			auto mutation = mutations.at(9);
			AssertThat(mutation->nodes.size(), Equals(2));
			AssertThat(mutation->nodes, Contains(Change<NodePtr>(p->a8, p->a8, ChangeType::Mutated, p->root(), 0)));
			AssertThat(mutation->nodes, Contains(Change<NodePtr>(p->b8, p->b8, ChangeType::Mutated, p->root(), 1)));
		});
	});
});