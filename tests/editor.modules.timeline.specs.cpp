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

		auto assertProperties = [&](const std::vector<QModelIndex> nodeIndices)
		{
			for (auto&& nodeIndex : nodeIndices)
			{
				auto node = model->nodeFromIndex(nodeIndex);
				LOG->debug("Asserting properties for node {}", Core::prop<std::string>(*node, "$Title", 0));

				// first rows are the child nodes, then the rest of the rows are the properties
				size_t propertyIndex = p->current().childCount(*node);
				AssertThat(model->rowCount(nodeIndex), Equals(node->properties().size() + propertyIndex));
				for (auto& prop : node->properties())
				{
					AssertThat(model->propertyFromIndex(model->index(propertyIndex, 0, nodeIndex)), Equals(prop.get()));
					AssertThat(model->roundTripPropertyValueFromIndex(model->index(propertyIndex, 1, nodeIndex)), Equals(prop->getPropertyValue(0)));
					propertyIndex++;
				}
			}
		};

		it("has added nodes and properties", [&]()
		{
			p->applyMutationsTo(0);
			AssertThat(model->data(model->index(0, 0), Qt::DisplayRole).toString().toStdString(), Equals("a"));
			AssertThat(model->data(model->index(1, 0), Qt::DisplayRole).toString().toStdString(), Equals("b"));
			AssertThat(model->data(model->index(2, 0), Qt::DisplayRole).toString().toStdString(), Equals("c"));
			assertProperties({ model->index(0, 0), model->index(1, 0), model->index(2, 0) });
		});

		it("has removed nodes", [&]()
		{
			p->applyMutationsTo(1);
			AssertThat(model->data(model->index(0, 0), Qt::DisplayRole).toString().toStdString(), Equals("a"));
			AssertThat(model->data(model->index(1, 0), Qt::DisplayRole).toString().toStdString(), Equals("c"));
			assertProperties({ model->index(0, 0), model->index(1, 0) });
		});

		it("has added nodes after undo", [&]()
		{
			p->applyMutationsTo(2);
			AssertThat(model->data(model->index(0, 0), Qt::DisplayRole).toString().toStdString(), Equals("a"));
			AssertThat(model->data(model->index(1, 0), Qt::DisplayRole).toString().toStdString(), Equals("b"));
			AssertThat(model->data(model->index(2, 0), Qt::DisplayRole).toString().toStdString(), Equals("c"));
			assertProperties({ model->index(0, 0), model->index(1, 0), model->index(2, 0) });
		});

		it("has mutated properties", [&]()
		{
			p->applyMutationsTo(3);
			AssertThat(model->data(model->index(0, 0), Qt::DisplayRole).toString().toStdString(), Equals("a"));
			AssertThat(model->data(model->index(1, 0), Qt::DisplayRole).toString().toStdString(), Equals("b"));
			AssertThat(model->data(model->index(2, 0), Qt::DisplayRole).toString().toStdString(), Equals("c"));
			assertProperties({ model->index(0, 0), model->index(1, 0), model->index(2, 0) });
		});

		it("can reparent from root to lower", [&]()
		{
			// Select a and c
			p->applyMutationsTo(7);
			oldSelection.append(model->index(0, 0));
			oldSelection.append(model->index(2, 0));

			p->applyMutation(8);
			AssertThat(model->data(model->index(0, 0), Qt::DisplayRole).toString().toStdString(), Equals("c"));
			AssertThat(model->data(model->index(0, 0, model->index(0, 0)), Qt::DisplayRole).toString().toStdString(), Equals("b"));
			AssertThat(model->data(model->index(1, 0, model->index(0, 0)), Qt::DisplayRole).toString().toStdString(), Equals("a"));

			AssertThat(newSelection.size(), Equals(2));
			auto a = newSelection.at(0);
			auto b = newSelection.at(1);
			AssertThat(model->data(newSelection.at(0), Qt::DisplayRole).toString().toStdString(), Equals("a") || Equals("c"));
			AssertThat(model->data(newSelection.at(1), Qt::DisplayRole).toString().toStdString(), Equals("a") || Equals("c"));
		});

		it("can reparent from lower to root", [&]()
		{
			// Select a and c
			p->applyMutationsTo(7);
			oldSelection.append(model->index(0, 0));
			oldSelection.append(model->index(2, 0));

			p->applyMutationsFromTo(8, 9);
			AssertThat(model->data(model->index(0, 0), Qt::DisplayRole).toString().toStdString(), Equals("a"));
			AssertThat(model->data(model->index(1, 0), Qt::DisplayRole).toString().toStdString(), Equals("b"));
			AssertThat(model->data(model->index(2, 0), Qt::DisplayRole).toString().toStdString(), Equals("c"));

			AssertThat(newSelection.size(), Equals(2));
			AssertThat(model->data(newSelection.at(0), Qt::DisplayRole).toString().toStdString(), Equals("a") || Equals("c"));
			AssertThat(model->data(newSelection.at(1), Qt::DisplayRole).toString().toStdString(), Equals("a") || Equals("c"));

			assertProperties({ model->index(0, 0), model->index(1, 0), model->index(2, 0) });
		});
	
		it("has inserted nodes", [&]()
		{
			p->applyMutationsTo(12);
			AssertThat(model->data(model->index(0, 0), Qt::DisplayRole).toString().toStdString(), Equals("a"));
			AssertThat(model->data(model->index(1, 0), Qt::DisplayRole).toString().toStdString(), Equals("between_ab"));
			AssertThat(model->data(model->index(2, 0), Qt::DisplayRole).toString().toStdString(), Equals("b"));
			AssertThat(model->data(model->index(3, 0), Qt::DisplayRole).toString().toStdString(), Equals("c"));
			assertProperties({ model->index(0, 0), model->index(1, 0), model->index(2, 0), model->index(3, 0) });
		});

		it("has renamed a node", [&]()
		{
			// Select a and c
			p->applyMutationsTo(12);
			oldSelection.append(model->index(0, 0));
			oldSelection.append(model->index(3, 0));

			p->applyMutationsTo(13);
			AssertThat(model->data(model->index(0, 0), Qt::DisplayRole).toString().toStdString(), Equals("a!"));
	
			AssertThat(newSelection.size(), Equals(2));
			AssertThat(model->data(newSelection.at(0), Qt::DisplayRole).toString().toStdString(), Equals("a!") || Equals("c"));
			AssertThat(model->data(newSelection.at(1), Qt::DisplayRole).toString().toStdString(), Equals("a!") || Equals("c"));
	
			assertProperties({ model->index(0, 0), model->index(1, 0), model->index(2, 0), model->index(3, 0) });
		});

		it("has added property", [&]()
		{
			p->applyMutationsTo(14);
			assertProperties({ model->index(0, 0), model->index(1, 0), model->index(2, 0), model->index(3, 0) });
		});

		it("has removed property", [&]()
		{
			p->applyMutationsTo(15);
			assertProperties({ model->index(0, 0), model->index(1, 0), model->index(2, 0), model->index(3, 0) });
		});
	
		it("has reparented nodes", [&]()
		{
			p->applyMutationsTo(27);
			AssertThat(model->data(model->index(2, 0), Qt::DisplayRole).toString().toStdString(), Equals("c"));
			AssertThat(model->data(model->index(0, 0, model->index(2, 0)), Qt::DisplayRole).toString().toStdString(), Equals("b"));
			AssertThat(model->data(model->index(0, 0, model->index(0, 0, model->index(2, 0))), Qt::DisplayRole).toString().toStdString(), Equals("a"));

			assertProperties({ model->index(2, 0), model->index(0, 0, model->index(2, 0)), model->index(0, 0, model->index(0, 0, model->index(2, 0))) });
		});
	});
});