#include "delegate.h"
#include "row_editor.h"
#include "widget.h"
#include "../model.h"

using Core::Project;
using Core::NodePtr;
using Core::PropertyPtr;
using Editor::Modules::Timeline::Model;
using Editor::Modules::Timeline::Keyframer::Delegate;
using Editor::Modules::Timeline::Keyframer::RowEditor;
using Editor::Modules::Timeline::Keyframer::Widget;

using ModelItemRoles = Model::ModelItemRoles;
using ModelItemDataType = Model::ModelItemDataType;

Delegate::Delegate(Project& project, const QSortFilterProxyModel& proxy, const Model& model)
	: project_(project)
	, proxy_(proxy)
	, model_(model)
{
}

QWidget* Delegate::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	auto type = static_cast<ModelItemDataType>(proxy_.data(index, static_cast<int>(ModelItemRoles::Type)).value<int>());

	RowEditor* editor;
	switch (type)
	{
	case ModelItemDataType::Node:
	{
		editor = RowEditor::makeEditor(const_cast<Delegate&>(*this), project_, model_, parent, proxy_.data(index, static_cast<int>(ModelItemRoles::Data)).value<NodePtr>());
		break;
	}
	case ModelItemDataType::Property:
	{
		editor = RowEditor::makeEditor(const_cast<Delegate&>(*this), project_, model_, parent, proxy_.data(index, static_cast<int>(ModelItemRoles::Data)).value<PropertyPtr>());
		break;
	}
	}

	connect(editor, &QObject::destroyed, this, [=](QObject*) { editors_.erase(editor); });

	editors_.insert(editor);
	return editor;
}

void Delegate::resetSelection()
{
	for (auto&& widget : selected()) widget->setSelected(false);
}

void Delegate::setSelected(Widget* widget, bool selected)
{
	auto s = this->selected();
	if (selected)
	{
		if (s.find(widget) != end(s)) return;
		widget->setSelected(true);
	}
	else
	{
		if (s.find(widget) == end(s)) return;
		widget->setSelected(false);
	}
}

bool Delegate::isSelected(Widget* widget) const
{
	auto s = selected();
	return s.find(widget) != end(s);
}

const std::unordered_set<Widget*> Delegate::widgets() const
{
	std::unordered_set<Widget*> result;
	for (auto&& editor : editors_)
	{
		for (auto&& widget : editor->widgets()) result.insert(widget);
	}
	return result;
}

const std::unordered_set<Widget*> Delegate::selected() const
{
	std::unordered_set<Widget*> result;
	for (auto&& editor : editors_)
	{
		for (auto&& widget : editor->widgets())
		{
			if (widget->isSelected()) result.insert(widget);
		}
	}
	return result;
}