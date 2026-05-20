#pragma once
#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <functional>
#include <iterator>
#include <memory>
#include <set>
#include <unordered_map>
#include <utility>
#include <vector>

template<typename T, typename Compare = std::less<T>>
class FibonacciHeap {
public:
    struct Node {
        T key;
        int degree;
        bool mark;

        Node* parent;
        Node* child;
        Node* left;
        Node* right;

        template<typename U>
        explicit Node(U&& k)
            : key(std::forward<U>(k)),
              degree(0),
              mark(false),
              parent(nullptr),
              child(nullptr),
              left(this),
              right(this) {}
    };

private:
    using IterSnapshot = std::shared_ptr<std::vector<Node*>>;

public:
    class iterator {
    public:
        using value_type = T;
        using reference = T&;
        using pointer = T*;
        using difference_type = std::ptrdiff_t;
        using iterator_category = std::forward_iterator_tag;

        iterator() : nodes_(nullptr), index_(0) {}

        reference operator*() const {
            assert(!isEnd());
            return (*nodes_)[index_]->key;
        }

        pointer operator->() const {
            assert(!isEnd());
            return &((*nodes_)[index_]->key);
        }

        iterator& operator++() {
            ++index_;
            return *this;
        }

        iterator operator++(int) {
            iterator tmp = *this;
            ++(*this);
            return tmp;
        }

        bool operator==(const iterator& other) const {
            if (isEnd() && other.isEnd()) {
                return true;
            }

            return nodes_ == other.nodes_ && index_ == other.index_;
        }

        bool operator!=(const iterator& other) const {
            return !(*this == other);
        }

    private:
        friend class FibonacciHeap;

        iterator(IterSnapshot nodes, std::size_t index)
            : nodes_(std::move(nodes)), index_(index) {}

        bool isEnd() const {
            return !nodes_ || index_ >= nodes_->size();
        }

        IterSnapshot nodes_;
        std::size_t index_;
    };

    class const_iterator {
    public:
        using value_type = T;
        using reference = const T&;
        using pointer = const T*;
        using difference_type = std::ptrdiff_t;
        using iterator_category = std::forward_iterator_tag;

        const_iterator() : nodes_(nullptr), index_(0) {}

        reference operator*() const {
            assert(!isEnd());
            return (*nodes_)[index_]->key;
        }

        pointer operator->() const {
            assert(!isEnd());
            return &((*nodes_)[index_]->key);
        }

        const_iterator& operator++() {
            ++index_;
            return *this;
        }

        const_iterator operator++(int) {
            const_iterator tmp = *this;
            ++(*this);
            return tmp;
        }

        bool operator==(const const_iterator& other) const {
            if (isEnd() && other.isEnd()) {
                return true;
            }

            return nodes_ == other.nodes_ && index_ == other.index_;
        }

        bool operator!=(const const_iterator& other) const {
            return !(*this == other);
        }

    private:
        friend class FibonacciHeap;

        const_iterator(IterSnapshot nodes, std::size_t index)
            : nodes_(std::move(nodes)), index_(index) {}

        bool isEnd() const {
            return !nodes_ || index_ >= nodes_->size();
        }

        IterSnapshot nodes_;
        std::size_t index_;
    };

    FibonacciHeap()
        : minNode_(nullptr),
          n_(0),
          comp_(Compare()) {}

    explicit FibonacciHeap(const Compare& comp)
        : minNode_(nullptr),
          n_(0),
          comp_(comp) {}

    FibonacciHeap(const FibonacciHeap& other)
        : minNode_(nullptr),
          n_(0),
          comp_(other.comp_) {
        copyFrom_(other);
    }

    FibonacciHeap(FibonacciHeap&& other) noexcept
        : minNode_(other.minNode_),
          n_(other.n_),
          comp_(std::move(other.comp_)) {
        other.minNode_ = nullptr;
        other.n_ = 0;
    }

    FibonacciHeap& operator=(const FibonacciHeap& other) {
        if (this != &other) {
            FibonacciHeap tmp(other);
            swap_(tmp);
        }

        return *this;
    }

    FibonacciHeap& operator=(FibonacciHeap&& other) noexcept {
        if (this != &other) {
            clear();

            minNode_ = other.minNode_;
            n_ = other.n_;
            comp_ = std::move(other.comp_);

            other.minNode_ = nullptr;
            other.n_ = 0;
        }

        return *this;
    }

    ~FibonacciHeap() {
        clear();
    }

    bool isEmpty() const {
        return minNode_ == nullptr;
    }

    std::size_t size() const {
        return n_;
    }

    template<typename U>
    Node* insert(U&& key) {
        Node* x = new Node(std::forward<U>(key));
        insertIntoRootList_(x);
        ++n_;
        return x;
    }

    const T& getMin() const {
        assert(minNode_ && "getMin() on empty heap");
        return minNode_->key;
    }

    T extractMin() {
        assert(minNode_ && "extractMin() on empty heap");

        Node* z = minNode_;

        if (z->child) {
            std::vector<Node*> kids;
            Node* c = z->child;

            do {
                kids.push_back(c);
                c = c->right;
            } while (c != z->child);

            for (Node* x : kids) {
                removeFromList_(x);
                x->parent = nullptr;
                x->mark = false;
                insertIntoRootList_(x);
            }

            z->child = nullptr;
            z->degree = 0;
        }

        Node* next = z->right;
        removeFromList_(z);

        if (next == z) {
            minNode_ = nullptr;
        } else {
            minNode_ = next;
            consolidate_();
        }

        T result = std::move(z->key);
        delete z;
        --n_;

        return result;
    }

    template<typename U>
    void decreaseKey(Node* x, U&& newKey) {
        assert(x && "decreaseKey: null node");
        assert(!comp_(x->key, newKey)
               && "decreaseKey: new key must not be greater");

        x->key = std::forward<U>(newKey);

        Node* y = x->parent;

        if (y && comp_(x->key, y->key)) {
            cut_(x, y);
            cascadingCut_(y);
        }

        if (minNode_ && comp_(x->key, minNode_->key)) {
            minNode_ = x;
        }
    }

    void clear() {
        if (!minNode_) {
            n_ = 0;
            return;
        }

        std::vector<Node*> all;
        collectAllNodes_(all);

        minNode_ = nullptr;
        n_ = 0;

        for (Node* node : all) {
            delete node;
        }
    }

    iterator begin() {
        return iterator(makeIterationSnapshot_(), 0);
    }

    iterator end() {
        return iterator();
    }

    const_iterator begin() const {
        return const_iterator(makeIterationSnapshot_(), 0);
    }

    const_iterator end() const {
        return const_iterator();
    }

    const_iterator cbegin() const {
        return const_iterator(makeIterationSnapshot_(), 0);
    }

    const_iterator cend() const {
        return const_iterator();
    }

private:
    Node* minNode_;
    std::size_t n_;
    Compare comp_;

    void swap_(FibonacciHeap& other) {
        using std::swap;

        swap(minNode_, other.minNode_);
        swap(n_, other.n_);
        swap(comp_, other.comp_);
    }

    bool less_(const T& a, const T& b) const {
        return comp_(a, b);
    }

    IterSnapshot makeIterationSnapshot_() const {
        auto nodes = std::make_shared<std::vector<Node*>>();
        collectAllNodes_(*nodes);
        return nodes;
    }

    void insertIntoRootList_(Node* x) {
        x->parent = nullptr;
        x->mark = false;

        if (!minNode_) {
            minNode_ = x;
            x->left = x;
            x->right = x;
            return;
        }

        x->left = minNode_;
        x->right = minNode_->right;
        minNode_->right->left = x;
        minNode_->right = x;

        if (less_(x->key, minNode_->key)) {
            minNode_ = x;
        }
    }

    static void removeFromList_(Node* x) {
        x->left->right = x->right;
        x->right->left = x->left;
        x->left = x;
        x->right = x;
    }

    void linkTrees_(Node* y, Node* x) {
        removeFromList_(y);

        y->parent = x;
        y->mark = false;

        if (!x->child) {
            x->child = y;
            y->left = y;
            y->right = y;
        } else {
            Node* c = x->child;

            y->left = c;
            y->right = c->right;
            c->right->left = y;
            c->right = y;
        }

        ++x->degree;
    }

    void consolidate_() {
        if (!minNode_) {
            return;
        }

        std::size_t maxDegree =
            static_cast<std::size_t>(std::log2(std::max<std::size_t>(1, n_)))
            + 3;

        std::vector<Node*> degreeTable(maxDegree, nullptr);
        std::vector<Node*> roots;

        Node* current = minNode_;

        do {
            roots.push_back(current);
            current = current->right;
        } while (current != minNode_);

        for (Node* root : roots) {
            Node* x = root;
            std::size_t degree = static_cast<std::size_t>(x->degree);

            while (degree >= degreeTable.size()) {
                degreeTable.resize(degreeTable.size() * 2 + 1, nullptr);
            }

            while (degreeTable[degree] != nullptr) {
                Node* y = degreeTable[degree];

                if (less_(y->key, x->key)) {
                    std::swap(x, y);
                }

                linkTrees_(y, x);
                degreeTable[degree] = nullptr;

                degree = static_cast<std::size_t>(x->degree);

                while (degree >= degreeTable.size()) {
                    degreeTable.resize(degreeTable.size() * 2 + 1, nullptr);
                }
            }

            degreeTable[degree] = x;
        }

        minNode_ = nullptr;

        for (Node* x : degreeTable) {
            if (!x) {
                continue;
            }

            x->left = x;
            x->right = x;

            if (!minNode_) {
                minNode_ = x;
            } else {
                x->left = minNode_;
                x->right = minNode_->right;
                minNode_->right->left = x;
                minNode_->right = x;

                if (less_(x->key, minNode_->key)) {
                    minNode_ = x;
                }
            }
        }
    }

    void cut_(Node* x, Node* y) {
        assert(x->parent == y);

        if (y->child == x) {
            if (x->right != x) {
                y->child = x->right;
            } else {
                y->child = nullptr;
            }
        }

        removeFromList_(x);
        --y->degree;

        x->parent = nullptr;
        x->mark = false;

        insertIntoRootList_(x);
    }

    void cascadingCut_(Node* y) {
        Node* z = y->parent;

        if (!z) {
            return;
        }

        if (!y->mark) {
            y->mark = true;
        } else {
            cut_(y, z);
            cascadingCut_(z);
        }
    }

    void collectAllNodes_(std::vector<Node*>& out) const {
        if (!minNode_) {
            return;
        }

        std::set<Node*> visited;
        std::vector<Node*> stack;
        std::vector<Node*> roots;

        Node* current = minNode_;

        do {
            roots.push_back(current);
            current = current->right;
        } while (current != minNode_);

        for (Node* root : roots) {
            stack.push_back(root);
        }

        while (!stack.empty()) {
            Node* x = stack.back();
            stack.pop_back();

            if (!x || visited.count(x)) {
                continue;
            }

            visited.insert(x);
            out.push_back(x);

            if (x->child) {
                Node* child = x->child;

                do {
                    stack.push_back(child);
                    child = child->right;
                } while (child != x->child);
            }
        }
    }

    void copyFrom_(const FibonacciHeap& other) {
        if (!other.minNode_) {
            minNode_ = nullptr;
            n_ = 0;
            return;
        }

        std::vector<Node*> oldNodes;
        other.collectAllNodes_(oldNodes);

        std::unordered_map<Node*, Node*> mapping;
        mapping.reserve(oldNodes.size());

        for (Node* oldNode : oldNodes) {
            Node* newNode = new Node(oldNode->key);

            newNode->degree = oldNode->degree;
            newNode->mark = oldNode->mark;

            mapping[oldNode] = newNode;
        }

        for (Node* oldNode : oldNodes) {
            Node* newNode = mapping[oldNode];

            newNode->parent = oldNode->parent ? mapping[oldNode->parent] : nullptr;
            newNode->child = oldNode->child ? mapping[oldNode->child] : nullptr;
            newNode->left = oldNode->left ? mapping[oldNode->left] : newNode;
            newNode->right = oldNode->right ? mapping[oldNode->right] : newNode;
        }

        minNode_ = mapping.at(other.minNode_);
        n_ = other.n_;
    }
};
