#include "node.h"

Node::Node(const Node& x) : self_(x.self_->copy())
{ }

void draw(const Node& x)
{
    x.self_->draw();
}

bool operator==(const Node& x, const std::string& other)
{
    return x.self_->equals(other);
}

