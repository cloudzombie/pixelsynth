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
		NodePtr a3, a4, a5;

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
		});

		it("should have emitted mutations", [&]()
		{
			AssertThat(mutations.size(), Equals(6));
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

		it("should emit changed connectors", [&]()
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
	});
});