#include "static.h"
#include <core/mutation-info.h>

using namespace bandit;
#include "test-utils.h"

go_bandit([]() {
	describe("mutation:", [&]()
	{
		std::unique_ptr<Project> p;
		std::vector<std::shared_ptr<MutationInfo>> mutations;

		NodePtr a0, b0, c0;
		NodePtr a3, a4, a5, a6, b6;

		before_each([&]()
		{
			mutations.clear();

			p = std::make_unique<Project>();
			p->setMutationCallback([&mutations](auto& mutationInfo) { mutations.emplace_back(mutationInfo); });

			// 0 = add nodes
			p->mutate([&](auto& mut)
			{
				a0 = makeNode(hash("TestNode"), "a");
				b0 = makeNode(hash("TestNode"), "b");
				c0 = makeNode(hash("TestNode"), "c");
				mut.append(nullptr, { a0, b0, c0 });
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
		});

		it("should have emitted mutations", [&]()
		{
			AssertThat(mutations.size(), Equals(8));
		});

		it("should emit added nodes", [&]()
		{
			auto mutation = mutations.at(0);
			AssertThat(mutation->nodes.size(), Equals(3));
			AssertThat(mutation->nodes[a0].added, Has().All().EqualTo(a0));
			AssertThat(mutation->nodes[b0].added, Has().All().EqualTo(b0));
			AssertThat(mutation->nodes[c0].added, Has().All().EqualTo(c0));
		});

		it("should emit removed nodes", [&]()
		{
			auto mutation = mutations.at(1);
			AssertThat(mutation->nodes.size(), Equals(1));
			AssertThat(mutation->nodes[b0].removed, Has().All().EqualTo(b0));
		});

		it("should emit nodes re-added after undo", [&]()
		{
			auto mutation = mutations.at(2);
			AssertThat(mutation->nodes.size(), Equals(1));
			AssertThat(mutation->nodes[b0].added, Has().All().EqualTo(b0));
		});

		it("should emit changed properties", [&]()
		{
			auto mutation = mutations.at(3);
			AssertThat(mutation->nodes.size(), Equals(1));
			AssertThat(mutation->nodes[a0].mutated.begin()->second, Equals(a3));

			AssertThat(mutation->properties.size(), Equals(1));
			AssertThat(mutation->properties[a0].mutated.find(prop(a0, "int"))->second, Equals(prop(a3, "int")));
		});

		it("should emit added connectors", [&]()
		{
			auto mutation = mutations.at(4);
			AssertThat(mutation->nodes.size(), Equals(1));
			AssertThat(mutation->nodes[a3].mutated.begin()->second, Equals(a4));

			AssertThat(mutation->connectorMetadata.size(), Equals(1));
			AssertThat(mutation->connectorMetadata[a3].added, Has().All().EqualTo(connector(a4, "Foo")));
		});

		it("should emit removed connectors", [&]()
		{
			auto mutation = mutations.at(5);
			AssertThat(mutation->nodes.size(), Equals(1));
			AssertThat(mutation->nodes[a4].mutated.begin()->second, Equals(a5));

			AssertThat(mutation->connectorMetadata.size(), Equals(1));
			AssertThat(mutation->connectorMetadata[a4].removed, Has().All().EqualTo(connector(a4, "Foo")));
		});

		it("should emit added connections", [&]()
		{
			auto mutation = mutations.at(6);
			AssertThat(mutation->connections.added.size(), Equals(1));
			AssertThat((*mutation->connections.added.begin())->connection(), Equals(make_tuple(a6, connector(a6, "Out"), b6, connector(b6, "In"))));
		});

		it("should emit removed connections", [&]()
		{
			auto mutation = mutations.at(7);
			AssertThat(mutation->connections.removed.size(), Equals(1));
			AssertThat((*mutation->connections.removed.begin())->connection(), Equals(make_tuple(a6, connector(a6, "Out"), b6, connector(b6, "In"))));
		});
	});
});