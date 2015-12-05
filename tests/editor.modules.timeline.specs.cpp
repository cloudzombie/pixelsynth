#include "static.h"
#include "mutationproject.h"

using namespace bandit;
#include "test-utils.h"
#include "testnode.h"
#include "modeltest.h"

#include <editor-lib/modules/timeline/model.h>

go_bandit([]() {
	describe("editor.modules.timeline:", []()
	{
		std::unique_ptr<MutationProject> p;
		std::unique_ptr<Editor::Modules::Timeline::Model> model;
		QModelIndexList oldSelection, newSelection;

		NodePtr a0, b0, c0;

		before_each([&]()
		{
			p = std::make_unique<MutationProject>();
			model = std::make_unique<Editor::Modules::Timeline::Model>();
			oldSelection.clear();
			newSelection.clear();
			p->setMutationCallback([&](auto mutationInfo) { newSelection = model->apply(mutationInfo, oldSelection); oldSelection = newSelection; });
		});

		auto qtTestModel = [&]()
		{
			auto m = std::make_shared<ModelTest>(model.get());
		};

		auto assertProperties = [&](const QModelIndex nodeIndex)
		{
			auto node = model->nodeFromIndex(nodeIndex);
			LOG->debug("Asserting properties for node {}", Core::prop<std::string>(*node, "$Title", 0));

			// first rows are the child nodes, then the rest of the rows are the properties
			size_t propertyIndex = p->current().childCount(*node);
			AssertThat(model->rowCount(nodeIndex), Equals(node->properties().size() + propertyIndex));
			for (auto& prop : node->properties())
			{
				AssertThat(model->propertyFromIndex(model->index(propertyIndex, 0, nodeIndex)), Equals(prop));
				AssertThat(model->roundTripPropertyValueFromIndex(model->index(propertyIndex, 1, nodeIndex)), Equals(prop->getPropertyValue(0)));
				propertyIndex++;
			}
		};

		// Walk the tree and construct appropriate model indices
		// This should match the generated model exactly
		auto assertModel = [&]()
		{
			for (auto node : p->current().nodes())
			{
				if (node == p->current().root()) continue;

				std::stack<int> childIndices;
				auto nodeTitle = prop<std::string>(*node, "$Title", 0);

				// Figure out the child indices starting from the node and working our way up to the root
				NodePtr parent;
				do
				{
					parent = p->current().parent(*node);
					if (parent) childIndices.push(p->current().childIndex(*node));
					node = parent;
				} while (parent);

				// Now we can build a nested QModelIndex based on the child indices
				auto index = QModelIndex();
				while (childIndices.size())
				{
					auto row = childIndices.top();
					index = model->index(row, 0, index);
					childIndices.pop();
				}

				// The model at this index should match the title of the node, if everything went well :)
				AssertThat(model->data(index, Qt::DisplayRole).toString().toStdString(), Equals(nodeTitle));

				// Also, all properties should be present
				assertProperties(index);
			}
		};

		it("always matches the document", [&]()
		{
			for (size_t t = 0; t < MutationProject::NUM_MUTATIONS; t++)
			{
				p->applyMutation(t);
				assertModel();
			}
		});

		it("can hold selection after reparent from root to lower", [&]()
		{
			// Select a and c
			p->applyMutationsTo(7);
			oldSelection.append(model->index(0, 0));
			oldSelection.append(model->index(2, 0));

			p->applyMutation(8);
			AssertThat(newSelection.size(), Equals(2));
			auto a = newSelection.at(0);
			auto b = newSelection.at(1);
			AssertThat(model->data(newSelection.at(0), Qt::DisplayRole).toString().toStdString(), Equals("a") || Equals("c"));
			AssertThat(model->data(newSelection.at(1), Qt::DisplayRole).toString().toStdString(), Equals("a") || Equals("c"));
		});

		it("can hold selection after reparent from lower to root", [&]()
		{
			// Select a and c
			p->applyMutationsTo(7);
			oldSelection.append(model->index(0, 0));
			oldSelection.append(model->index(2, 0));

			p->applyMutationsFromTo(8, 9);
			AssertThat(newSelection.size(), Equals(2));
			AssertThat(model->data(newSelection.at(0), Qt::DisplayRole).toString().toStdString(), Equals("a") || Equals("c"));
			AssertThat(model->data(newSelection.at(1), Qt::DisplayRole).toString().toStdString(), Equals("a") || Equals("c"));
		});
	
		it("has hold selection after renaming a node", [&]()
		{
			// Select a and c
			p->applyMutationsTo(12);
			oldSelection.append(model->index(0, 0));
			oldSelection.append(model->index(3, 0));

			p->applyMutationsTo(13);
			AssertThat(newSelection.size(), Equals(2));
			AssertThat(model->data(newSelection.at(0), Qt::DisplayRole).toString().toStdString(), Equals("a!") || Equals("c"));
			AssertThat(model->data(newSelection.at(1), Qt::DisplayRole).toString().toStdString(), Equals("a!") || Equals("c"));
		});
	});
});