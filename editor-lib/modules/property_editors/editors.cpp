#include "editors.h"

using Editor::Modules::PropertyEditors::EditorVec3;

EditorVec3::EditorVec3(QWidget *parent)
	: QWidget(parent)
	, layout_(new QHBoxLayout(this))
{
	setAutoFillBackground(true);

	layout_->setSpacing(1);
	layout_->setMargin(1);

	x_ = new QDoubleSpinBox();
	y_ = new QDoubleSpinBox();
	z_ = new QDoubleSpinBox();
	x_->setMinimum(-100000);
	y_->setMinimum(-100000);
	z_->setMinimum(-100000);
	x_->setMaximum(100000);
	y_->setMaximum(100000);
	z_->setMaximum(100000);

	layout_->addWidget(x_);
	layout_->addWidget(y_);
	layout_->addWidget(z_);

	connect(x_, SIGNAL(valueChanged(double)), this, SLOT(xChanged(double)));
	connect(y_, SIGNAL(valueChanged(double)), this, SLOT(yChanged(double)));
	connect(z_, SIGNAL(valueChanged(double)), this, SLOT(zChanged(double)));

	setLayout(layout_);

	setFocusProxy(x_);
	z_->installEventFilter(this);
}

void EditorVec3::xChanged(double d) { value_.setX(d); }
void EditorVec3::yChanged(double d) { value_.setY(d); }
void EditorVec3::zChanged(double d) { value_.setZ(d); }

void EditorVec3::setValue(const QVector3D& value)
{
	value_ = value;
	x_->setValue(value_.x());
	y_->setValue(value_.y());
	z_->setValue(value_.z());
}

bool EditorVec3::eventFilter(QObject* object, QEvent* event)
{
	if (object == z_ && event->type() == QEvent::FocusOut)
	{
		x_->setFocus();
	}
	return false;
}