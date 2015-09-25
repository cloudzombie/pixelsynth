// Based on Sean Parent stuff

#include <iostream>
#include <vector>
#include <memory>
#include <cassert>
#include <map>
#include "tree.h"

using namespace std;

struct Box {
    Box(string title): title(title) {}
    string title { "MyBox" };
};

bool operator==(const Box& a, const string& title) { return a.title == title; }

struct Renderer {
};

template <typename T>
bool operator==(const T& a, const string& title) { return false; }

struct Connector {
    enum class Type { Input, Output } type;
    string title;
};

typedef pair<Connector, Connector> Connection;

typedef float Frame;
struct Property {
    map<Frame, int> keys;
};

void draw(const Box& x)
{
    cout << "Box: " << x.title << endl;
}

void draw(const Renderer& x)
{
    cout << "Renderer" << endl;
}

void draw(const Connection& c)
{
    cout << "connection" << endl;
}

#include "node.h"

Connection connect(const Connector& a, const Connector& b)
{
    return Connection(a, b);
}

class Document {
public:
    tree<Node> nodes_;
    vector<Connection> connections_;
    map<Node, Property> properties_;
};

void draw(const Document& x)
{
    for (auto& n: x.nodes_) draw(n);
    for (auto& n: x.connections_) draw(n);
    cout << "---------------" << endl;
}

using history_t = vector<Document>;

void commit(history_t& x) { assert(x.size()); x.push_back(x.back()); }
void undo(history_t& x) { assert(x.size()); x.pop_back(); }
Document& current(history_t& x) { assert(x.size()); return x.back(); }

int main() {
    history_t h(1);
    auto top = current(h).nodes_.begin();
    current(h).nodes_.insert(top, Box("root"));
    auto rnd = current(h).nodes_.insert(top, Renderer());
    auto ch1 = current(h).nodes_.append_child(rnd, Box("child 1"));
    current(h).nodes_.append_child(rnd, Box("child 2"));
    draw(current(h));
    commit(h);

    Connector a, b;
    current(h).connections_.emplace_back(connect(a, b));

    ch1 = find(begin(current(h).nodes_), end(current(h).nodes_), string("child 1"));
    current(h).nodes_.append_child(ch1, Box("child 3"));
    current(h).nodes_.append_child(ch1, Box("child 4"));
    draw(current(h));
    commit(h);

    ch1 = find(begin(current(h).nodes_), end(current(h).nodes_), string("child 1"));
    auto ch2 = find(begin(current(h).nodes_), end(current(h).nodes_), string("child 2"));

    auto selbeg = current(h).nodes_.begin_leaf(ch1);
    auto selend = current(h).nodes_.end(current(h).nodes_.end_leaf(ch1));
    current(h).nodes_.reparent(ch2, selbeg, selend);

    draw(current(h));
    undo(h);
    undo(h);

    draw(current(h));

    return 0;
}