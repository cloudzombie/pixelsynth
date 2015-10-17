#include "ostream.h"
#include "utils.h"

using namespace Core;

std::ostream& Core::operator<<(std::ostream& out, const Node& n)
{
	if (&n == nullptr) out << "Node(nullptr)";
	else out << "Node(" << prop<std::string>(n, "$Title", 0) << "|" << &n << ")";
	return out;
}

std::ostream& Core::operator<<(std::ostream& out, const Property& p)
{
	if (&p == nullptr) out << "Property(nullptr)";
	else out << "Property(" << p.metadata().title() << "|" << &p << ")";
	return out;
}

std::ostream& Core::operator<<(std::ostream& out, const PropertyMetadata& p)
{
	if (&p == nullptr)
	{
		out << "PropertyMetadata(nullptr)";
		return out;
	}

	out << "PropertyMetadata(" << p.title() << "|" << &p << ")";
	return out;
}

std::ostream& Core::operator<<(std::ostream& out, const ConnectorMetadata& c)
{
	if (&c == nullptr)
	{
		out << "ConnectorMetadata(nullptr)";
		return out;
	}

	std::string type;
	switch (c.type())
	{
	case ConnectorType::Input: type = "Input"; break;
	case ConnectorType::Output: type = "Output"; break;
	}

	out << "ConnectorMetadata(" << c.title() << ", " << type << "|" << &c << ")";
	return out;
}
