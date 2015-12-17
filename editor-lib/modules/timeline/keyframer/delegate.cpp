#include "delegate.h"
#include "row_editor.h"
#include "widget.h"
#include "../model.h"

#include "editors/node_editor.h"
#include "editors/property_editor.h"

#include <core/project.h>

using Core::Project;
using Core::NodePtr;
using Core::PropertyPtr;
using Editor::Modules::Timeline::Model;
using Editor::Modules::Timeline::Keyframer::Delegate;
using Editor::Modules::Timeline::Keyframer::RowEditor;
using Editor::Modules::Timeline::Keyframer::TrimEdge;
using Editor::Modules::Timeline::Keyframer::Widget;
using Editor::Modules::Timeline::Keyframer::Editors::NodeEditor;
using Editor::Modules::Timeline::Keyframer::Editors::PropertyEditor;

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

	connect(editor, &RowEditor::widgetCreated, this, &Delegate::widgetCreated);
	connect(editor, &QObject::destroyed, this, [=](QObject*) { editors_.erase(editor); });

	editor->initializeWidgets();

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

void Delegate::setRubberBandSelection(QRect globalRect)
{
	for (auto&& widget : widgets())
	{
		auto globalNodeRect = QRect(0, 0, widget->width(), widget->height()).translated(widget->parentWidget()->mapToGlobal(widget->pos()));

		if (globalRect.intersects(globalNodeRect))
		{
			if (!isSelected(widget))
			{
				dragSelected_.insert(widget);
				setSelected(widget, true);
			}
		}
		else
		{
			if (dragSelected_.find(widget) != end(dragSelected_))
			{
				dragSelected_.erase(widget);
				setSelected(widget, false);
			}
		}
	}
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

void Delegate::widgetCreated(Widget* widget)
{
	connect(widget, &Widget::clicked, this, &Delegate::widgetClicked);
	connect(widget, &Widget::dragged, this, &Delegate::widgetDragged);
	connect(widget, &Widget::trimmed, this, &Delegate::widgetTrimmed);
	connect(widget, &Widget::released, this, &Delegate::widgetReleased);
}

void Delegate::widgetClicked(bool multiSelect)
{
	auto widget = qobject_cast<Widget*>(sender());
	if (!multiSelect)
	{
		resetSelection();
		setSelected(widget, true);
	}
	else
	{
		setSelected(widget, !isSelected(widget));
	}
}

void Delegate::widgetDragged(int offset)
{
	std::unordered_set<Widget*> alreadyProcessed;

	for (auto&& node : project_.current().nodes())
	{
		auto ne = editorFor(node);
		if (ne && ne->node() == node && (ne->isSelected() || sender() == ne->widget()))
		{
			ne->applyOffset(offset, alreadyProcessed);
		}

		for (auto&& prop : node->properties())
		{
			auto pe = editorFor(prop);
			if (pe && pe->property() == prop)
			{
				pe->applyOffset(offset, alreadyProcessed, [sender = sender()](Widget* w) { return w->isSelected() || sender == w; });
			}
		}
	}
}

void Delegate::widgetTrimmed(const int offset, TrimEdge edge)
{
	std::unordered_set<Widget*> alreadyProcessed;

	for (auto&& node : project_.current().nodes())
	{
		auto ne = editorFor(node);
		if (ne && ne->node() == node && (ne->isSelected() || sender() == ne->widget()))
		{
			ne->applyTrim(offset, edge, alreadyProcessed);
		}
	}
}

void Delegate::widgetReleased()
{
	bool anyDirty = false;
	for (auto&& node : project_.current().nodes())
	{
		auto editor = editorFor(node);
		if (editor && editor->isDirty())
		{
			anyDirty = true;
			break;
		}
	}

	if (!anyDirty) return;

	project_.mutate([&](Core::Document::Builder& mut)
	{
		for (auto&& node : project_.current().nodes())
		{
			auto editor = editorFor(node);
			if (editor) editor->applyMutations(mut);
		}
	}, "Change visibility");

}

void Delegate::applyInDocumentOrder(std::unordered_set<Widget*> widgets, std::function<void(Widget*)> fn) const
{
	// Not particularly efficient, meh
	for (auto&& node : project_.current().nodes())
	{
		for (auto&& w : widgets)
		{
			auto ne = qobject_cast<NodeEditor*>(w->editor());
			if (ne && ne->node() == node) fn(w);
		}

		for (auto&& prop : node->properties())
		{
			for (auto&& w : widgets)
			{
				auto pe = qobject_cast<PropertyEditor*>(w->editor());
				if (pe && pe->property() == prop) fn(w);
			}
		}
	}
}

const std::unordered_set<Widget*> Delegate::selectionAnd(Widget* additionalWidget) const
{
	auto widgets = selected();
	if (additionalWidget) widgets.insert(additionalWidget);
	return widgets;
}

NodeEditor* Delegate::editorFor(NodePtr node) const
{
	for (auto&& editor : editors_)
	{
		auto ne = qobject_cast<NodeEditor*>(editor);
		if (ne && ne->node() == node) return ne;
	}
	return nullptr;
}

PropertyEditor* Delegate::editorFor(PropertyPtr property) const
{
	for (auto&& editor : editors_)
	{
		auto pe = qobject_cast<PropertyEditor*>(editor);
		if (pe && pe->property() == property) return pe;
	}
	return nullptr;
}