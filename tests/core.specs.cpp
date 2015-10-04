#include "static.h"

using namespace bandit;
#include "test-utils.h"
#include "testnode.h"

go_bandit([]() {
	describe("project:", []()
	{
		std::unique_ptr<Project> p;

		before_each([&]()
		{
			p = std::make_unique<Project>();
		});

		it("allows adding nodes", [&]()
		{
			AssertThat(p->current().totalChildCount(p->root()), Equals(0));
			p->mutate([](auto& mut) { mut.append({ makeNode(hash("TestNode"), "a") }); });
			AssertThat(p->current().totalChildCount(p->root()), Equals(1));
			p->mutate([](auto& mut) { mut.append({ makeNode(hash("TestNode"), "b") }); });
			AssertThat(p->current().totalChildCount(p->root()), Equals(2));
			AssertThat(findNode(*p, "a") == nullptr, Equals(false));
			AssertThat(findNode(*p, "b") == nullptr, Equals(false));
			p->undo();
			AssertThat(p->current().totalChildCount(p->root()), Equals(1));
			AssertThat(findNode(*p, "a") == nullptr, Equals(false));
			AssertThat(findNode(*p, "b") == nullptr, Equals(true));
			p->undo();
			AssertThat(p->current().totalChildCount(p->root()), Equals(0));
			AssertThat(findNode(*p, "a") == nullptr, Equals(true));
			AssertThat(findNode(*p, "b") == nullptr, Equals(true));
		});

		it("allows grouping nodes", [&]()
		{
			AssertThat(p->current().totalChildCount(p->root()), Equals(0));
			p->mutate([](auto& mut)
			{
				mut.append({ makeNode(hash("TestNode"), "a") });
				mut.append({ makeNode(hash("TestNode"), "b") });
				mut.append({ makeNode(hash("TestNode"), "c") });
			});
			AssertThat(p->current().totalChildCount(p->root()), Equals(3));
			p->mutate([](auto& mut) { mut.append({ makeNode(hash("TestNode"), "g1") }); });
			auto g1 = findNode(*p, "g1");
			auto b = findNode(*p, "b");
			auto c = findNode(*p, "c");
			p->mutate([&](auto& mut) { mut.reparent(g1, { b, c }); });
			AssertThat(p->current().parent(b), Equals(g1));
			AssertThat(p->current().parent(c), Equals(g1));
			AssertThat(p->current().totalChildCount(g1), Equals(2));
			AssertThat(p->current().totalChildCount(p->root()), Equals(4));
			p->mutate([&](auto& mut)
			{
				auto g1 = findNode(*p, "g1");
				mut.eraseChildren({ g1 });
				mut.erase({ g1 });
			});
			AssertThat(p->current().totalChildCount(p->root()), Equals(1));

			p->undo();
			AssertThat(p->current().totalChildCount(g1), Equals(2));
			AssertThat(p->current().totalChildCount(p->root()), Equals(4));
		});

		it("can reset", [&]()
		{
			const int NUM_ITERATIONS = 10;
			for (int t = 0; t < NUM_ITERATIONS; t++) p->mutate([](auto& mut) { mut.append({ makeNode(hash("TestNode"), "a") }); });

			AssertThat(p->current().totalChildCount(p->root()), Equals(NUM_ITERATIONS));

			p = std::make_unique<Project>();
			AssertThat(p->current().totalChildCount(p->root()), Equals(0));
			for (int t = 0; t < NUM_ITERATIONS; t++) p->mutate([](auto& mut) { mut.append({ makeNode(hash("TestNode"), "a") }); });
			AssertThat(p->current().totalChildCount(p->root()), Equals(NUM_ITERATIONS));
		});
	});

	describe("node:", []()
	{
		std::unique_ptr<Project> p;

		before_each([&]()
		{
			p = std::make_unique<Project>();
			p->mutate([&](auto& mut) { mut.append({ makeNode(hash("TestNode"), "a") }); });
		});

		it("can delete a node", [&]()
		{
			auto node = findNode(*p, "a");
			p->mutate([&](auto& mut) { mut.erase({ node }); });

			AssertThat(findNode(*p, "a") == nullptr, Equals(true));
			p->undo();
			AssertThat(findNode(*p, "a") == nullptr, Equals(false));
		});

		it("can change a property", [&]()
		{
			auto node_a = findNode(*p, "a");
			p->mutate([&](Document::Builder& mut)
			{
				mut.mutate(node_a, [&](Node::Builder& node)
				{
					node.mutateProperty(hash("$Title"), [&](Property::Builder& prop) { prop.set(0, "a2"); });
				});
			});
			auto node_a2 = findNode(*p, "a2");
			AssertThat(node_a->properties(), !Equals(node_a2->properties()));

			AssertThat(findNode(*p, "a") == nullptr, Equals(true));
			AssertThat(findNode(*p, "a2") == nullptr, Equals(false));
			p->undo();
			AssertThat(findNode(*p, "a") == nullptr, Equals(false));
			AssertThat(findNode(*p, "a2") == nullptr, Equals(true));
		});

		it("can animate a property", [&]()
		{
			p->mutate([&](Document::Builder& mut)
			{
				TestNode::addKeyframes(mut, findNode(*p, "a"));
			});
			TestNode::assertKeyframes(findNode(*p, "a"));
		});

		it("can add a connector", [&]()
		{
			p->mutate([&](Document::Builder& mut)
			{
				auto node_a = findNode(*p, "a");
				mut.mutate(node_a, [&](Node::Builder& node)
				{
					node.addConnector(ConnectorMetadata::Builder("Test", ConnectorType::Output));
				});
			});
			AssertThat(connector(findNode(*p, "a"), "Test") == nullptr, Equals(false));
			p->undo();
			AssertThat(connector(findNode(*p, "a"), "Test") == nullptr, Equals(true));
			p->redo();
			AssertThat(connector(findNode(*p, "a"), "Test") == nullptr, Equals(false));
		});
	});

	describe("connection:", [&]()
	{
		std::unique_ptr<Project> p;

		before_each([&]()
		{
			p = std::make_unique<Project>();
			p->mutate([&](auto& mut)
			{
				mut.append({ makeNode(hash("TestNode"), "a") });
				mut.append({ makeNode(hash("TestNode"), "b") });
			});
			p->mutate([&](auto& mut)
			{
				auto node_a = findNode(*p, "a");
				auto node_b = findNode(*p, "b");
				mut.connect(std::make_shared<Connection>(make_tuple(node_a, connector(node_a, "Out"), node_b, connector(node_b, "In"))));
			});
		});

		it("can connect nodes", [&]()
		{
			auto node_a = findNode(*p, "a");
			auto node_b = findNode(*p, "b");
			AssertThat(p->current().connections().size(), Equals(1));
			AssertThat(p->current().connections()[0]->outputNode() == node_a, Equals(true));

			p->mutate([&](Document::Builder& mut)
			{
				mut.mutate(node_a, [&](Node::Builder& node)
				{
					node.mutateProperty(hash("$Title"), [&](Property::Builder& prop) { prop.set(0, "a2"); });
				});
			});
			auto node_a2 = findNode(*p, "a2");

			AssertThat(p->current().connections()[0]->outputNode() == node_a2, Equals(true));
			AssertThat(p->current().connections()[0]->inputNode() == node_b, Equals(true));
			p->undo();
			AssertThat(p->current().connections()[0]->outputNode() == node_a, Equals(true));
			AssertThat(p->current().connections()[0]->inputNode() == node_b, Equals(true));
			p->undo();
			AssertThat(p->current().connections().size(), Equals(0));
		});

		describe("can disconnect nodes on deletion", [&]()
		{
			before_each([&]()
			{
				auto node_a = findNode(*p, "a");
				p->mutate([&](auto& mut) { mut.erase({ node_a }); });
			});

			it("should disconnect and reconnect", [&]()
			{
				AssertThat(p->current().connections().size(), Equals(0));
				p->undo();
				AssertThat(p->current().connections().size(), Equals(1));
				auto node_a = findNode(*p, "a");
				AssertThat(p->current().connections()[0]->outputNode() == node_a, Equals(true));
			});
		});
	});
});
