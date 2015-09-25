#pragma once
#include <utility>
#include <memory>
#include <iostream>

template <typename T>
void draw(const T& x)
{
    std::cout << "draw" << std::endl;
}

class Node {
public:
    Node() = default;

    template <typename T>
    Node(T x): self_(new Model<T>(std::move(x)))
    {}

    Node(const Node& x);
    Node(Node&&) noexcept = default;

    Node& operator=(const Node& x)
    { Node tmp(x); *this = std::move(tmp); return *this; }
    Node& operator=(Node&&) noexcept = default;

    friend void draw(const Node& x);
    friend bool operator==(const Node& x, const std::string& other);

private:
    struct Concept {
        virtual ~Concept() = default;
        virtual Concept* copy() const = 0;
        virtual void draw() const = 0;
        virtual bool equals(const std::string& other) const = 0;
    };

    template <typename T>
    struct Model: Concept {
        Model(T x): data_(std::move(x)) {}
        Concept* copy() const override { return new Model(*this); }
        void draw() const override
        { ::draw(data_); }

        bool equals(const std::string& other) const override
        {
            return data_ == other;
        }

        T data_;
    };

    std::unique_ptr<const Concept> self_;
};
