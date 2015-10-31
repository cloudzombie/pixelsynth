#include "mutationproject.h"
#include "test-utils.h"

void MutationProject::applyMutationsTo(size_t maxMutation)
{
	for (size_t mutationIndex = 0; mutationIndex <= maxMutation; mutationIndex++) applyMutation(mutationIndex);
}

void MutationProject::applyMutationsFromTo(size_t start, size_t end)
{
	for (size_t mutationIndex = start; mutationIndex <= end; mutationIndex++) applyMutation(mutationIndex);
}

void MutationProject::applyMutation(size_t mutationIndex)
{
	NodePtr a {}, b {}, c {};
	if (mutationIndex > 0)
	{
		a = this->a[mutationIndex - 1];
		b = this->b[mutationIndex - 1];
		c = this->c[mutationIndex - 1];
	}

	switch (mutationIndex)
	{
	case 0:
		// 0 = add nodes
		this->mutate([&](auto& mut)
		{
			a = makeNode(hash("TestNode"), "a");
			b = makeNode(hash("TestNode"), "b");
			c = makeNode(hash("TestNode"), "c");
			mut.append({ a, b, c });
		});
		break;

	case 1:
		// 1 = remove node
		this->mutate([&](auto& mut)
		{
			mut.erase({ b });
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
			mut.mutate(a, [&](Node::Builder& node)
			{
				node.mutateProperty(hash("int"), [&](Property::Builder& prop) { prop.set(100, 50); });
				node.mutateProperty(hash("double"), [&](Property::Builder& prop) { prop.set(100, 50.0); });
				node.mutateProperty(hash("vec2"), [&](Property::Builder& prop) { prop.set(100, glm::vec2(50, 50)); });
				node.mutateProperty(hash("vec3"), [&](Property::Builder& prop) { prop.set(100, glm::vec3(50, 50, 50)); });
				node.mutateProperty(hash("string"), [&](Property::Builder& prop) { prop.set(100, "bob"); });
			});
		});
		break;

	case 4:
		// 4 = add connector
		this->mutate([&](Document::Builder& mut)
		{
			mut.mutate(a, [&](Node::Builder& node)
			{
				node.addConnector(ConnectorMetadata::Builder("Foo", ConnectorType::Output));
			});
		});
		break;

	case 5:
		// 5 = undo add connector
		this->undo();
		break;

	case 6:
		// 6 = connect a.out to b.in
		this->mutate([&](Document::Builder& mut)
		{
			mut.connect(std::make_shared<Connection>(make_tuple(a, connector(*a, "Out"), b, connector(*b, "In"))));
		});
		break;

	case 7:
		// 7 = undo connect
		this->undo();
		break;

	case 8:
		// 8 = reparent a and b under c, in reverse order
		this->mutate([&](Document::Builder& mut)
		{
			mut.reparent(c, { b, a });
		});
		break;

	case 9:
		// 9 = undo reparent
		this->undo();
		break;

	case 10:
		// 10 = set property double to animated
		this->mutate([&](Document::Builder& mut)
		{
			mut.mutate(a, [&](Node::Builder& node)
			{
				node.mutateProperty(hash("double"), [&](Property::Builder& prop) { prop.setAnimated(true); });
			});
		});
		break;

	case 11:
		// 11 = set property int to animated
		this->mutate([&](Document::Builder& mut)
		{
			mut.mutate(a, [&](Node::Builder& node)
			{
				node.mutateProperty(hash("int"), [&](Property::Builder& prop) { prop.setAnimated(true); });
			});
		});
		break;

	case 12:
		// 12 = insert new node between_ab
		this->mutate([&](auto& mut)
		{
			auto node = makeNode(hash("TestNode"), "between_ab");
			mut.insertBefore(b, { node });
		});
		break;

	case 13:
		// 13 = rename node a to a!
		this->mutate([&](Document::Builder& mut)
		{
			mut.mutate(a, [&](Node::Builder& node)
			{
				node.mutateProperty(hash("$Title"), [&](Property::Builder& prop) { prop.set(0, "a!"); });
			});
		});
		break;

	case 14:
		// 14 = add a property
		this->mutate([&](Document::Builder& mut)
		{
			mut.mutate(a, [&](Node::Builder& node)
			{
				node.addProperty(PropertyMetadata::Builder("new_property").ofType<int>());
			});
		});
		break;

	case 15:
		// 15 = undo add property
		this->undo();
		break;

	case 16:
		// 16 = rename a!! back to a
		this->undo();
		break;

	case 17:
		// 17 = remove between_ab
		this->undo();
		break;

	case 18:
		// 18 = reparent b under a
		this->mutate([&](Document::Builder& mut)
		{
			mut.reparent(a, { b });
		});
		break;

	case 19:
		// 19 = reparent a under c, should also keep b reparented under a
		this->mutate([&](Document::Builder& mut)
		{
			mut.reparent(c, { a });
		});
		break;

	case 20:
		// 20 = undo 19
		this->undo();
		break;

	case 21:
		// 21 = redo 19
		this->redo();
		break;

	case 22:
		// 22 = undo 19 (reparenting)
		this->undo();
		break;

	case 23:
		// 22 = undo 18 (reparenting now completely undone)
		this->undo();
		break;

	case 24:
		// 24 = add two nodes in separate mutation calls
		this->mutate({
			[&](auto& mut) { mut.insertBefore(b, { makeNode(hash("TestNode"), "between_ab") }); },
			[&](auto& mut) { mut.insertBefore(b, { makeNode(hash("TestNode"), "between_ab2") }); }
		});
		break;

	case 25:
		// 25 = undo 24
		this->undo();
		break;

	case 26:
		// 26 = redo 24
		this->redo();
		break;

	case 27:
		throw new std::logic_error("No more migrations");
	}

	auto findHistory = [this, mutationIndex](std::array<NodePtr, 500>& arr, std::vector<std::string> names)
	{
		for (auto&& name: names)
		{
			auto&& ptr = findNode(*this, name);
			if (ptr.get()) arr[mutationIndex] = ptr;
		}
	};

	findHistory(this->a, { "a", "a!" });
	findHistory(this->b, { "b" });
	findHistory(this->c, { "c" });
	findHistory(this->between_ab, { "between_ab" });
	findHistory(this->between_ab2, { "between_ab2" });
}
