#include "static.h"
#include <core/project.h>
#include <core/metadata.h>
#include <core/factory.h>

using namespace bandit;
using namespace Core;

struct TestNode
{
	static PropertyMetadataCollection propertyMetadata()
	{
		return
		{
			PropertyMetadata::Builder("Title")
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
	builder->mutateProperty(hash("Title"))->set(0, title);
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
			{
				auto mut = p->mutate();
				mut->append(nullptr, { makeNode(hash("TestNode"), "a") });
			}
			AssertThat(p->current().childCount(), Equals(1));
			{
				auto mut = p->mutate();
				mut->append(nullptr, { makeNode(hash("TestNode"), "b") });
			}
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
			{
				auto mut = p->mutate();
				mut->append(nullptr, { makeNode(hash("TestNode"), "a") });
				mut->append(nullptr, { makeNode(hash("TestNode"), "b") });
				mut->append(nullptr, { makeNode(hash("TestNode"), "c") });
			}
			AssertThat(p->current().childCount(), Equals(3));
			{
				auto mut = p->mutate();
				mut->append(nullptr, { makeNode(hash("TestNode"), "g1") });
			}
			auto g1 = findNode(*p, "g1");
			auto b = findNode(*p, "b");
			auto c = findNode(*p, "c");
			{
				auto mut = p->mutate();
				mut->reparent(g1, { b, c });
			}
			AssertThat(p->current().parent(b), Equals(g1));
			AssertThat(p->current().parent(c), Equals(g1));
			AssertThat(p->current().childCount(g1), Equals(2));
			AssertThat(p->current().childCount(), Equals(4));
			{
				auto mut = p->mutate();
				auto g1 = findNode(*p, "g1");
				mut->eraseChildren({ g1 });
				mut->erase({ g1 });
			}
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
			auto mut = p->mutate();
			mut->append(nullptr, { makeNode(hash("TestNode"), "a") });
		});

		it("can delete a node", [&]()
		{
			{
				auto node = findNode(*p, "a");
				auto pmut = p->mutate();
				pmut->erase({ node });
			}

			AssertThat(findNode(*p, "a") == nullptr, Equals(true));
			p->undo();
			AssertThat(findNode(*p, "a") == nullptr, Equals(false));
		});

		it("can change a property", [&]()
		{
			auto node_a = findNode(*p, "a");
			{
				auto pmut = p->mutate();
				auto nmut = pmut->mutate(node_a);
				auto propmut = nmut->mutateProperty(hash("Title"));
				propmut->set(0, "a2");
			}
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
			{
				auto mut = p->mutate();
				mut->append(nullptr, { makeNode(hash("TestNode"), "a") });
				mut->append(nullptr, { makeNode(hash("TestNode"), "b") });
			}
			{
				auto mut = p->mutate();
				auto node_a = findNode(*p, "a");
				auto node_b = findNode(*p, "b");
				mut->connect(std::make_shared<Connection>(make_tuple(node_a, connector(node_a, "Out"), node_b, connector(node_b, "In"))));
			}
		});

		it("can connect nodes", [&]()
		{
			auto node_a = findNode(*p, "a");
			AssertThat(p->current().connections().size(), Equals(1));
			AssertThat(outputNode(p->current().connections()[0]) == node_a, Equals(true));

			{
				auto pmut = p->mutate();
				auto nmut = pmut->mutate(node_a);
				auto propmut = nmut->mutateProperty(hash("Title"));
				propmut->set(0, "a2");
			}
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
				auto pmut = p->mutate();
				pmut->erase({ node_a });
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
});

int main(int argc, char* argv[])
{
	DefineNode(TestNode);

	// Run the tests.
	return bandit::run(argc, argv);
}