#include "keyframe_header.h"
#include "model.h"

#include <core/document.h>

using Core::Document;
using Editor::Modules::Timeline::KeyframeHeader;
using Editor::Modules::Timeline::Model;

KeyframeHeader::KeyframeHeader(const Model& model, QWidget* parent)
	: QHeaderView(Qt::Horizontal, parent)
	, model_(model)
	, document_(nullptr)
{
	widget_ = new QWidget(this);
	widget_->setStyleSheet("background-color: #ff0;");
	widget_->show();

	auto update = [&](const Document* prev, const Document* cur)
	{
		document_ = cur;
		widget_->resize(document_->settings().visibility.second, height());
	};

	connect(&model, &Model::documentMutated, this, update);

	connect(this, &QHeaderView::sectionResized, this, [=]()
	{
		setSectionResizeMode(0, QHeaderView::ResizeToContents);
		update(document_, document_);
	});
}

void KeyframeHeader::update()
{
}