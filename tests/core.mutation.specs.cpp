#include "static.h"

using namespace bandit;
#include "test-utils.h"

template <typename T>
using Change = MutationInfo::Change<T>;
using ChangeType = MutationInfo::ChangeType;

go_bandit([]() {
	describe("mutation:", [&]()
	{
		std::unique_ptr<Project> p;
		std::vector<std::shared_ptr<MutationInfo>> mutations;

		NodePtr a0, b0, c0;
		NodePtr a3, a4, a5, a6, b6;
		NodePtr a8, b8, c8;

		before_each([&]()
		{
			mutations.clear();

			p = std::make_unique<Project>();
			p->setMutationCallback([&mutations](auto mutationInfo) { mutations.emplace_back(mutationInfo); });

			// 0 = add nodes
			p->mutate([&](auto& mut)
			{
				a0 = makeNode(hash("TestNode"), "a");
				b0 = makeNode(hash("TestNode"), "b");
				c0 = makeNode(hash("TestNode"), "c");
				mut.append({ a0, b0, c0 });
			});

			// 1 = remove node
			p->mutate([&](auto& mut)
			{
				mut.erase({ b0 });
			});

			// 2 = undo remove
			p->undo();

			// 3 = set properties
			p->mutate([&](Document::Builder& mut)
			{
				mut.mutate(a0, [&](Node::Builder& node)
				{
					node.mutateProperty(hash("int"), [&](Property::Builder& prop) { prop.set(100, 50); });
					node.mutateProperty(hash("double"), [&](Property::Builder& prop) { prop.set(100, 50.0); });
					node.mutateProperty(hash("vec2"), [&](Property::Builder& prop) { prop.set(100, glm::vec2(50, 50)); });
					node.mutateProperty(hash("vec3"), [&](Property::Builder& prop) { prop.set(100, glm::vec3(50, 50, 50)); });
					node.mutateProperty(hash("string"), [&](Property::Builder& prop) { prop.set(100, "bob"); });
				});
			});
			a3 = findNode(*p, "a");

			// 4 = add connector
			p->mutate([&](Document::Builder& mut)
			{
				mut.mutate(a3, [&](Node::Builder& node)
				{
					node.addConnector(ConnectorMetadata::Builder("Foo", ConnectorType::Output));
				});
			});
			a4 = findNode(*p, "a");

			// 5 = undo add connector
			p->undo();
			a5 = findNode(*p, "a");

			// 6 = connect a.out to b.in
			p->mutate([&](Document::Builder& mut)
			{
				auto node_a = findNode(*p, "a");
				auto node_b = findNode(*p, "b");
				mut.connect(std::make_shared<Connection>(make_tuple(node_a, connector(node_a, "Out"), node_b, connector(node_b, "In"))));
			});
			a6 = findNode(*p, "a");
			b6 = findNode(*p, "b");

			// 7 = undo connect
			p->undo();

			// 8 = reparent a and b under c
			p->mutate([&](Document::Builder& mut)
			{
				auto node_a = findNode(*p, "a");
				auto node_b = findNode(*p, "b");
				auto node_c = findNode(*p, "c");
				mut.reparent(node_c, { node_a, node_b });
			});
			a8 = findNode(*p, "a");
			b8 = findNode(*p, "b");
			c8 = findNode(*p, "c");

			// 9 = undo reparent
			p->undo();
		});

		it("should have emitted mutations", [&]()
		{
			AssertThat(mutations.size(), Equals(10));
		});

		it("should emit added nodes", [&]()
		{
			auto mutation = mutations.at(0);
			AssertThat(mutation->nodes.size(), Equals(3));
			AssertThat(mutation->nodes, Contains(Change<NodePtr>(nullptr, a0, ChangeType::Added, p->root())));
			AssertThat(mutation->nodes, Contains(Change<NodePtr>(nullptr, b0, ChangeType::Added, p->root())));
			AssertThat(mutation->nodes, Contains(Change<NodePtr>(nullptr, c0, ChangeType::Added, p->root())));
		});

		it("should emit removed nodes", [&]()
		{
			auto mutation = mutations.at(1);
			AssertThat(mutation->nodes.size(), Equals(1));
			AssertThat(mutation->nodes, Contains(Change<NodePtr>(b0, nullptr, ChangeType::Removed, p->root())));
		});

		it("should emit nodes re-added after undo", [&]()
		{
			auto mutation = mutations.at(2);
			AssertThat(mutation->nodes.size(), Equals(1));
			AssertThat(mutation->nodes, Contains(Change<NodePtr>(nullptr, b0, ChangeType::Added, p->root())));
		});

		it("should emit changed properties", [&]()
		{
			auto mutation = mutations.at(3);
			AssertThat(mutation->nodes.size(), Equals(1));
			AssertThat(mutation->nodes, Contains(Change<NodePtr>(a0, a3, ChangeType::Mutated, p->root())));

			AssertThat(mutation->properties.size(), Equals(5));
			AssertThat(mutation->properties, Contains(Change<PropertyPtr>(prop(a0, "int"), prop(a3, "int"), ChangeType::Mutated, a3)));
			AssertThat(mutation->properties, Contains(Change<PropertyPtr>(prop(a0, "double"), prop(a3, "double"), ChangeType::Mutated, a3)));
			AssertThat(mutation->properties, Contains(Change<PropertyPtr>(prop(a0, "vec2"), prop(a3, "vec2"), ChangeType::Mutated, a3)));
			AssertThat(mutation->properties, Contains(Change<PropertyPtr>(prop(a0, "vec3"), prop(a3, "vec3"), ChangeType::Mutated, a3)));
			AssertThat(mutation->properties, Contains(Change<PropertyPtr>(prop(a0, "string"), prop(a3, "string"), ChangeType::Mutated, a3)));
		});

		it("should emit added connectors", [&]()
		{
			auto mutation = mutations.at(4);
			AssertThat(mutation->nodes.size(), Equals(1));
			AssertThat(mutation->nodes, Contains(Change<NodePtr>(a3, a4, ChangeType::Mutated, p->root())));

			AssertThat(mutation->connectors.size(), Equals(1));
			AssertThat(mutation->connectors, Contains(Change<ConnectorMetadataPtr>(nullptr, connector(a4, "Foo"), ChangeType::Added, a4)));
		});

		it("should emit removed connectors", [&]()
		{
			auto mutation = mutations.at(5);
			AssertThat(mutation->nodes.size(), Equals(1));
			AssertThat(mutation->nodes, Contains(Change<NodePtr>(a4, a5, ChangeType::Mutated, p->root())));

			AssertThat(mutation->connectors.size(), Equals(1));
			AssertThat(mutation->connectors, Contains(Change<ConnectorMetadataPtr>(connector(a4, "Foo"), nullptr, ChangeType::Removed, a4)));
		});

		it("should emit added connections", [&]()
		{
			auto mutation = mutations.at(6);
			AssertThat(mutation->connections.size(), Equals(1));
			AssertThat(mutation->connections.at(0).cur->connection(), Equals(make_tuple(a6, connector(a6, "Out"), b6, connector(b6, "In"))));
		});

		it("should emit removed connections", [&]()
		{
			auto mutation = mutations.at(7);
			AssertThat(mutation->connections.size(), Equals(1));
			AssertThat(mutation->connections.at(0).prev->connection(), Equals(make_tuple(a6, connector(a6, "Out"), b6, connector(b6, "In"))));
		});

		it("should emit reparenting from root to lower", [&]()
		{
			auto mutation = mutations.at(8);
			AssertThat(mutation->nodes.size(), Equals(2));
			AssertThat(mutation->nodes, Contains(Change<NodePtr>(a8, a8, ChangeType::Mutated, c8)));
			AssertThat(mutation->nodes, Contains(Change<NodePtr>(b8, b8, ChangeType::Mutated, c8)));
		});

		it("should emit reparenting from lower to root", [&]()
		{
			auto mutation = mutations.at(9);
			AssertThat(mutation->nodes.size(), Equals(2));
			AssertThat(mutation->nodes, Contains(Change<NodePtr>(a8, a8, ChangeType::Mutated, p->root())));
			AssertThat(mutation->nodes, Contains(Change<NodePtr>(b8, b8, ChangeType::Mutated, p->root())));
		});
	});
});