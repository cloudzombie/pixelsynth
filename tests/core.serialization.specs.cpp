#include "static.h"
#include <core/serializer.h>

using namespace bandit;
#include "test-utils.h"
#include "testnode.h"

go_bandit([]() {
	describe("serializer:", [&]()
	{
		std::unique_ptr<Project> p;
		std::unique_ptr<Project> p2;

		before_each([&]()
		{
			p = std::make_unique<Project>();
			p->mutate([&](auto& mut)
			{
				mut.append({ makeNode(hash("TestNode"), "a") });
				mut.append({ makeNode(hash("TestNode"), "b") });
				mut.append({ makeNode(hash("TestNode"), "c") });
			});
			p->mutate([&](Document::Builder& mut)
			{
				auto node_a = findNode(*p, "a");
				auto node_b = findNode(*p, "b");
				auto node_c = findNode(*p, "c");
				mut.connect(std::make_shared<Connection>(make_tuple(node_a, connector(*node_a, "Out"), node_b, connector(*node_b, "In"))));
				TestNode::addKeyframes(mut, node_a);

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
			AssertThat(p2->current().connections()[0]->output() == connector(*node_a, "Out"), Equals(true));
			AssertThat(p2->current().connections()[0]->input() == connector(*node_b, "In"), Equals(true));

			AssertThat(connector(*node_c, "Test") == nullptr, Equals(false));

			TestNode::assertKeyframes(node_a);
		});
	});
});