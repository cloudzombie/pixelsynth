#include "ostream.h"
#include "utils.h"

using namespace Core;

std::ostream& Core::operator<<(std::ostream& out, const Node& n)
{
	out << "Node(" << prop<std::string>(n, "$Title", 0) << ")";
	return out;
}

std::ostream& Core::operator<<(std::ostream& out, const Property& p)
{
	out << "Property(" << p.metadata().title() << ")";
	return out;
}

std::ostream& Core::operator<<(std::ostream& out, const PropertyMetadata& p)
{
	out << "PropertyMetadata(" << p.title() << ")";
	return out;
}

std::ostream& Core::operator<<(std::ostream& out, const ConnectorMetadata& c)
{
	std::string type;
	switch (c.type())
	{
	case ConnectorType::Input: type = "Input"; break;
	case ConnectorType::Output: type = "Output"; break;
	}

	out << "ConnectorMetadata(" << c.title() << ", " << type << ")";
	return out;
}
