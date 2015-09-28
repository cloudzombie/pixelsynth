#include "static.h"
#include <core/project.h>
#include <core/metadata.h>

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

	static Metadata* metadata()
	{
		static auto m = Metadata
		{
			propertyMetadata()
		};
		return &m;
	}
};

template <typename T>
std::shared_ptr<Node> makeNode(std::string title)
{
	auto builder = std::make_shared<Node::Builder>(T::metadata());
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

		it("allows adding nodes and undoing", [&]()
		{
			AssertThat(p->current().nodes().size(), Equals(0));
			{
				auto mut = p->mutate();
				mut->nodes().insert(mut->nodes().begin(), makeNode<TestNode>("a"));
			}
			AssertThat(p->current().nodes().size(), Equals(1));
			{
				auto mut = p->mutate();
				mut->nodes().insert(mut->nodes().begin(), makeNode<TestNode>("b"));
			}
			AssertThat(p->current().nodes().size(), Equals(2));
			AssertThat(findNode(*p, "a") == nullptr, Equals(false));
			AssertThat(findNode(*p, "b") == nullptr, Equals(false));
			p->undo();
			AssertThat(p->current().nodes().size(), Equals(1));
			AssertThat(findNode(*p, "a") == nullptr, Equals(false));
			AssertThat(findNode(*p, "b") == nullptr, Equals(true));
			p->undo();
			AssertThat(p->current().nodes().size(), Equals(0));
			AssertThat(findNode(*p, "a") == nullptr, Equals(true));
			AssertThat(findNode(*p, "b") == nullptr, Equals(true));
		});
	});

	describe("node:", []()
	{
		std::unique_ptr<Project> p;

		before_each([&]()
		{
			p = std::make_unique<Project>();
			auto mut = p->mutate();
			mut->nodes().insert(mut->nodes().begin(), makeNode<TestNode>("a"));
		});

		it("can delete a node", [&]()
		{
			{
				auto node = findNode(*p, "a");
				auto pmut = p->mutate();
				//auto nmut = pmut->erase(node);
			}
		});

		it("can change a property", [&]()
		{
			{
				auto node = findNode(*p, "a");
				auto pmut = p->mutate();
				auto nmut = pmut->mutate(node);
				auto propmut = nmut->mutateProperty(hash("Title"));
				propmut->set(0, "b");
			}

			AssertThat(title(*(p->current().nodes().begin())), Equals("b"));
			p->undo();
			AssertThat(title(*(p->current().nodes().begin())), Equals("a"));
		});
	});
});

int main(int argc, char* argv[])
{
	// Run the tests.
	return bandit::run(argc, argv);
}