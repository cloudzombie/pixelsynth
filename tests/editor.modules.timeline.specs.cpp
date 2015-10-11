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

		NodePtr a0, b0, c0;

		before_each([&]()
		{
			p = std::make_unique<MutationProject>();
			model = std::make_unique<Editor::Modules::Timeline::Model>();
			p->setMutationCallback([&](auto mutationInfo) { model->apply(mutationInfo); });
		});

		auto qtTestModel = [&]()
		{
			auto m = std::make_shared<ModelTest>(model.get());
		};

		it("has added nodes", [&]()
		{
			p->applyMutationsTo(0);
			AssertThat(model->data(model->index(0, 0), Qt::DisplayRole).toString().toStdString(), Equals("a"));
			AssertThat(model->data(model->index(1, 0), Qt::DisplayRole).toString().toStdString(), Equals("b"));
			AssertThat(model->data(model->index(2, 0), Qt::DisplayRole).toString().toStdString(), Equals("c"));
		});

		it("has removed nodes", [&]()
		{
			p->applyMutationsTo(1);
			AssertThat(model->data(model->index(0, 0), Qt::DisplayRole).toString().toStdString(), Equals("a"));
			AssertThat(model->data(model->index(1, 0), Qt::DisplayRole).toString().toStdString(), Equals("c"));
		});

		it("has added nodes after undo", [&]()
		{
			p->applyMutationsTo(2);
			AssertThat(model->data(model->index(0, 0), Qt::DisplayRole).toString().toStdString(), Equals("a"));
			AssertThat(model->data(model->index(1, 0), Qt::DisplayRole).toString().toStdString(), Equals("b"));
			AssertThat(model->data(model->index(2, 0), Qt::DisplayRole).toString().toStdString(), Equals("c"));
		});

		it("can reparent from root to lower", [&]()
		{
			p->applyMutationsTo(8);
			AssertThat(model->data(model->index(0, 0), Qt::DisplayRole).toString().toStdString(), Equals("c"));
			AssertThat(model->data(model->index(0, 0, model->index(0, 0)), Qt::DisplayRole).toString().toStdString(), Equals("a"));
			AssertThat(model->data(model->index(1, 0, model->index(0, 0)), Qt::DisplayRole).toString().toStdString(), Equals("b"));
		});

		it("can reparent from lower to root", [&]()
		{
			p->applyMutationsTo(9);
			AssertThat(model->data(model->index(0, 0), Qt::DisplayRole).toString().toStdString(), Equals("a"));
			AssertThat(model->data(model->index(1, 0), Qt::DisplayRole).toString().toStdString(), Equals("b"));
			AssertThat(model->data(model->index(2, 0), Qt::DisplayRole).toString().toStdString(), Equals("c"));
		});
	});
});