#pragma once
#include <editor-lib/static.h>

BEGIN_NAMESPACE(Editor) BEGIN_NAMESPACE(Modules) BEGIN_NAMESPACE(PropertyEditors)

class EditorVec3: public QWidget
{
	Q_OBJECT
	Q_PROPERTY(QVector3D value READ value WRITE setValue USER true)

public:
	explicit EditorVec3(QWidget* parent);

	void setValue(const QVector3D& value);
	QVector3D value() const { return value_; }

private slots:
	void xChanged(double d);
	void yChanged(double d);
	void zChanged(double d);

private:
	bool eventFilter(QObject* object, QEvent* event) override;

	QVector3D value_;

	QHBoxLayout* layout_;
	QDoubleSpinBox* x_;
	QDoubleSpinBox* y_;
	QDoubleSpinBox* z_;
};

END_NAMESPACE(Editor) END_NAMESPACE(Modules) END_NAMESPACE(PropertyEditors)
