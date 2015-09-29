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
			PropertyMetadata::Builder("Title").ofType<std::string>()
		};
	}

	static ConnectorMetadataCollection connectorMetadata()
	{
		return
		{
			ConnectorMetadata::Builder("Out", ConnectorType::Output),
			ConnectorMetadata::Builder("In", ConnectorType::Input)
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

std::shared_ptr<Node> makeNode(Hash type, std::string title)
{
	auto builder = Factory::makeNode(type);
	builder->mutateProperty(hash("Title"), [&](auto& prop) { prop.set(0, title); });
	return std::make_shared<Node>(std::move(*builder));
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
			AssertThat(p->current().connections().size(), Equals(1));
			AssertThat(outputNode(p->current().connections()[0]) == node_a, Equals(true));

			p->mutate([&](Document::Builder& mut)
			{
				mut.mutate(node_a, [&](Node::Builder& node)
				{
					node.mutateProperty(hash("Title"), [&](Property::Builder& prop) { prop.set(0, "a2"); });
				});
			});
			auto node_a2 = findNode(*p, "a2");

			AssertThat(outputNode(p->current().connections()[0]) == node_a2, Equals(true));
			p->undo();
			AssertThat(outputNode(p->current().connections()[0]) == node_a, Equals(true));
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
				AssertThat(outputNode(p->current().connections()[0]) == node_a, Equals(true));
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

			AssertThat(findNode(*p2, "a") == nullptr, Equals(false));
			AssertThat(findNode(*p2, "b") == nullptr, Equals(false));
			AssertThat(findNode(*p2, "c") == nullptr, Equals(false));
		});
	});
});

int main(int argc, char* argv[])
{
	DefineNode(TestNode);

	// Run the tests.
	return bandit::run(argc, argv);
}