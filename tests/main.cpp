#include "static.h"
#include <core/project.h>
#include <core/metadata.h>
#include <core/factory.h>
#include <core/serializer.h>

using namespace bandit;
using namespace Core;

struct TestNode
{
	static PropertyMetadataCollection propertyMetadata()
	{
		return
		{
			PropertyMetadata::Builder("Title").ofType<std::string>().build(),
			PropertyMetadata::Builder("int").ofType<int>().build(),
			PropertyMetadata::Builder("double").ofType<double>().build(),
			PropertyMetadata::Builder("string").ofType<std::string>().build()
		};
	}

	static ConnectorMetadataCollection connectorMetadata()
	{
		return
		{
			ConnectorMetadata::Builder("Out", ConnectorType::Output).build(),
			ConnectorMetadata::Builder("In", ConnectorType::Input).build()
		};
	}

	static Metadata* metadata()
	{
		static auto m = Metadata
		{
			propertyMetadata(),
			connectorMetadata()
		};
		return &m;
	}
};

std::shared_ptr<Node> makeNode(HashValue type, std::string title)
{
	auto builder = Factory::makeNode(type);
	builder->mutateProperty(hash("Title"), [&](auto& prop) { prop.set(0, title); });
	return std::make_shared<Node>(std::move(*builder));
}

static void addKeyframes(Document::Builder& mut, NodePtr n)
{
	mut.mutate(n, [&](Node::Builder& node)
	{
		node.mutateProperty(hash("int"), [&](Property::Builder& prop) { prop.set(0, -500); prop.set(100, 500); });
		node.mutateProperty(hash("double"), [&](Property::Builder& prop) { prop.set(0, -500.0); prop.set(100, 500.0); });
		node.mutateProperty(hash("string"), [&](Property::Builder& prop) { prop.set(0, "a"); prop.set(100, "b"); });
	});
}

static void assertKeyframes(NodePtr n)
{
	AssertThat(n->prop(hash("int"))->get<int>(0), Equals(-500));
	AssertThat(n->prop(hash("int"))->get<int>(50), Equals(0));
	AssertThat(n->prop(hash("int"))->get<int>(100), Equals(500));

	AssertThat(n->prop(hash("double"))->get<double>(0), Equals(-500));
	AssertThat(n->prop(hash("double"))->get<double>(50), Equals(0));
	AssertThat(n->prop(hash("double"))->get<double>(100), Equals(500));

	AssertThat(n->prop(hash("string"))->get<std::string>(0), Equals("a"));
	AssertThat(n->prop(hash("string"))->get<std::string>(50), Equals("a"));
	AssertThat(n->prop(hash("string"))->get<std::string>(100), Equals("b"));
}

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
			AssertThat(p->current().childCount(), Equals(0));
			p->mutate([](auto& mut) { mut.append(nullptr, { makeNode(hash("TestNode"), "a") }); });
			AssertThat(p->current().childCount(), Equals(1));
			p->mutate([](auto& mut) { mut.append(nullptr, { makeNode(hash("TestNode"), "b") }); });
			AssertThat(p->current().childCount(), Equals(2));
			AssertThat(findNode(*p, "a") == nullptr, Equals(false));
			AssertThat(findNode(*p, "b") == nullptr, Equals(false));
			p->undo();
			AssertThat(p->current().childCount(), Equals(1));
			AssertThat(findNode(*p, "a") == nullptr, Equals(false));
			AssertThat(findNode(*p, "b") == nullptr, Equals(true));
			p->undo();
			AssertThat(p->current().childCount(), Equals(0));
			AssertThat(findNode(*p, "a") == nullptr, Equals(true));
			AssertThat(findNode(*p, "b") == nullptr, Equals(true));
		});

		it("allows grouping nodes", [&]()
		{
			AssertThat(p->current().childCount(), Equals(0));
			p->mutate([](auto& mut)
			{
				mut.append(nullptr, { makeNode(hash("TestNode"), "a") });
				mut.append(nullptr, { makeNode(hash("TestNode"), "b") });
				mut.append(nullptr, { makeNode(hash("TestNode"), "c") });
			});
			AssertThat(p->current().childCount(), Equals(3));
			p->mutate([](auto& mut) { mut.append(nullptr, { makeNode(hash("TestNode"), "g1") }); });
			auto g1 = findNode(*p, "g1");
			auto b = findNode(*p, "b");
			auto c = findNode(*p, "c");
			p->mutate([&](auto& mut) { mut.reparent(g1, { b, c }); });
			AssertThat(p->current().parent(b), Equals(g1));
			AssertThat(p->current().parent(c), Equals(g1));
			AssertThat(p->current().childCount(g1), Equals(2));
			AssertThat(p->current().childCount(), Equals(4));
			p->mutate([&](auto& mut)
			{
				auto g1 = findNode(*p, "g1");
				mut.eraseChildren({ g1 });
				mut.erase({ g1 });
			});
			AssertThat(p->current().childCount(), Equals(1));

			p->undo();
			AssertThat(p->current().childCount(g1), Equals(2));
			AssertThat(p->current().childCount(), Equals(4));
		});
	});

	describe("node:", []()
	{
		std::unique_ptr<Project> p;

		before_each([&]()
		{
			p = std::make_unique<Project>();
			p->mutate([&](auto& mut) { mut.append(nullptr, { makeNode(hash("TestNode"), "a") }); });
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
					node.mutateProperty(hash("Title"), [&](Property::Builder& prop) { prop.set(0, "a2"); });
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
				addKeyframes(mut, findNode(*p, "a"));
			});
			assertKeyframes(findNode(*p, "a"));
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
				mut.append(nullptr, { makeNode(hash("TestNode"), "a") });
				mut.append(nullptr, { makeNode(hash("TestNode"), "b") });
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
					node.mutateProperty(hash("Title"), [&](Property::Builder& prop) { prop.set(0, "a2"); });
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

	describe("serializer:", [&]()
	{
		std::unique_ptr<Project> p;
		std::unique_ptr<Project> p2;

		before_each([&]()
		{
			p = std::make_unique<Project>();
			p->mutate([&](auto& mut)
			{
				mut.append(nullptr, { makeNode(hash("TestNode"), "a") });
				mut.append(nullptr, { makeNode(hash("TestNode"), "b") });
				mut.append(nullptr, { makeNode(hash("TestNode"), "c") });
			});
			p->mutate([&](Document::Builder& mut)
			{
				auto node_a = findNode(*p, "a");
				auto node_b = findNode(*p, "b");
				auto node_c = findNode(*p, "c");
				mut.connect(std::make_shared<Connection>(make_tuple(node_a, connector(node_a, "Out"), node_b, connector(node_b, "In"))));
				addKeyframes(mut, node_a);

				mut.mutate(node_c, [&](Node::Builder& node)
				{
					node.addConnector(ConnectorMetadata::Builder("Test", ConnectorType::Output));
				});
			});
		});

		it("should serialize and deserialize", [&]()
		{
			std::stringstream s;
			{
				cereal::XMLOutputArchive archive(s);
				archive(*p);
			}

			p2 = std::make_unique<Project>();
			{
				cereal::XMLInputArchive archive(s);
				archive(*p2);
			}
			auto node_a = findNode(*p2, "a");
			auto node_b = findNode(*p2, "b");
			auto node_c = findNode(*p2, "c");

			AssertThat(node_a == nullptr, Equals(false));
			AssertThat(node_b == nullptr, Equals(false));
			AssertThat(node_c == nullptr, Equals(false));
			AssertThat(p2->current().connections().size(), Equals(1));

			AssertThat(p2->current().connections()[0]->outputNode() == node_a, Equals(true));
			AssertThat(p2->current().connections()[0]->inputNode() == node_b, Equals(true));
			AssertThat(p2->current().connections()[0]->output() == connector(node_a, "Out"), Equals(true));
			AssertThat(p2->current().connections()[0]->input() == connector(node_b, "In"), Equals(true));

			AssertThat(connector(node_c, "Test") == nullptr, Equals(false));

			assertKeyframes(node_a);
		});
	});
});

int main(int argc, char* argv[])
{
	DefineNode(TestNode);

	// Run the tests.
	return bandit::run(argc, argv);
}