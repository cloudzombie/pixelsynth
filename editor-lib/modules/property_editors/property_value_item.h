#pragma once
#include <editor-lib/static.h>

BEGIN_NAMESPACE(Editor) BEGIN_NAMESPACE(Modules) BEGIN_NAMESPACE(PropertyEditors)

class PropertyValueItem: public QStandardItem
{
public:
	explicit PropertyValueItem(const Core::Property* prop);

	void update(const Core::Property* prop);
	QVariant data(int role) const override;

	Core::PropertyValue roundTrip() const;

private:
	void setData(const QVariant& value, int role) override;

	const Core::Property* prop_;
};

END_NAMESPACE(Editor) END_NAMESPACE(Modules) END_NAMESPACE(PropertyEditors)
