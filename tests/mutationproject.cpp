#include "mutationproject.h"
#include "test-utils.h"

void MutationProject::applyMutationsTo(size_t maxMutation)
{
	for (size_t mutationIndex = 0; mutationIndex <= maxMutation; mutationIndex++)
	{
		switch (mutationIndex)
		{
		case 0:
			// 0 = add nodes
			this->mutate([&](auto& mut)
			{
				a0 = makeNode(hash("TestNode"), "a");
				b0 = makeNode(hash("TestNode"), "b");
				c0 = makeNode(hash("TestNode"), "c");
				mut.append({ a0, b0, c0 });
			});
			break;

		case 1:
			// 1 = remove node
			this->mutate([&](auto& mut)
			{
				mut.erase({ b0 });
			});
			break;

		case 2:
			// 2 = undo remove
			this->undo();
			break;

		case 3:
			// 3 = set properties
			this->mutate([&](Document::Builder& mut)
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
			a3 = findNode(*this, "a");
			break;

		case 4:
			// 4 = add connector
			this->mutate([&](Document::Builder& mut)
			{
				mut.mutate(a3, [&](Node::Builder& node)
				{
					node.addConnector(ConnectorMetadata::Builder("Foo", ConnectorType::Output));
				});
			});
			a4 = findNode(*this, "a");
			break;

		case 5:
			// 5 = undo add connector
			this->undo();
			a5 = findNode(*this, "a");
			break;

		case 6:
			// 6 = connect a.out to b.in
			this->mutate([&](Document::Builder& mut)
			{
				auto node_a = findNode(*this, "a");
				auto node_b = findNode(*this, "b");
				mut.connect(std::make_shared<Connection>(make_tuple(node_a, connector(node_a, "Out"), node_b, connector(node_b, "In"))));
			});
			a6 = findNode(*this, "a");
			b6 = findNode(*this, "b");
			break;

		case 7:
			// 7 = undo connect
			this->undo();
			break;

		case 8:
			// 8 = reparent a and b under c
			this->mutate([&](Document::Builder& mut)
			{
				auto node_a = findNode(*this, "a");
				auto node_b = findNode(*this, "b");
				auto node_c = findNode(*this, "c");
				mut.reparent(node_c, { node_a, node_b });
			});
			a8 = findNode(*this, "a");
			b8 = findNode(*this, "b");
			c8 = findNode(*this, "c");
			break;

		case 9:
			// 9 = undo reparent
			this->undo();
			break;

		case 10:
			throw new std::logic_error("No more migrations");
		}
	}
}
