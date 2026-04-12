#pragma once

#include <cassert>
#include <cmath>
#include <cstddef>
#include <functional>
#include <limits>
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

        explicit Node(const T& k)
            : key(k),
              degree(0),
              mark(false),
              parent(nullptr),
              child(nullptr),
              left(this),
              right(this) {}

        explicit Node(T&& k)
            : key(std::move(k)),
              degree(0),
              mark(false),
              parent(nullptr),
              child(nullptr),
              left(this),
              right(this) {}
    };

    FibonacciHeap()
        : minNode_(nullptr), n_(0), comp_(Compare()) {}

    explicit FibonacciHeap(const Compare& comp)
        : minNode_(nullptr), n_(0), comp_(comp) {}

    FibonacciHeap(const FibonacciHeap& other)
        : minNode_(nullptr), n_(0), comp_(other.comp_) {
        copyFrom_(other);
    }

    FibonacciHeap(FibonacciHeap&& other) noexcept
        : minNode_(nullptr), n_(0), comp_(std::move(other.comp_)) {
        swap_(other);
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
            comp_ = std::move(other.comp_);
            swap_(other);
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

    Node* insert(const T& key) {
        Node* x = new Node(key);
        insertIntoRootList_(x);
        ++n_;
        return x;
    }

    Node* insert(T&& key) {
        Node* x = new Node(std::move(key));
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

        T res = std::move(z->key);
        delete z;
        --n_;
        return res;
    }

    void decreaseKey(Node* x, const T& newKey) {
        assert(x && "decreaseKey: null node");
        assert(!comp_(x->key, newKey) && "decreaseKey: new key must not be greater");

        x->key = newKey;
        Node* y = x->parent;

        if (y && comp_(x->key, y->key)) {
            cut_(x, y);
            cascadingCut_(y);
        }

        if (minNode_ && comp_(x->key, minNode_->key)) {
            minNode_ = x;
        }
    }

    void decreaseKey(Node* x, T&& newKey) {
        assert(x && "decreaseKey: null node");
        assert(!comp_(x->key, newKey) && "decreaseKey: new key must not be greater");

        x->key = std::move(newKey);
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

        for (Node* p : all) {
            p->left = p->right = p;
            p->parent = p->child = nullptr;
            delete p;
        }
    }

private:
    Node* minNode_;
    std::size_t n_;
    Compare comp_;

    void swap_(FibonacciHeap& other) noexcept {
        std::swap(minNode_, other.minNode_);
        std::swap(n_, other.n_);
    }

    bool less_(const T& a, const T& b) const {
        return comp_(a, b);
    }

    void insertIntoRootList_(Node* x) {
        x->parent = nullptr;
        x->mark = false;

        if (!minNode_) {
            minNode_ = x;
            x->left = x->right = x;
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
        x->left = x->right = x;
    }

    void linkTrees_(Node* y, Node* x) {
        removeFromList_(y);
        y->parent = x;
        y->mark = false;

        if (!x->child) {
            x->child = y;
            y->left = y->right = y;
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
        if (!minNode_) return;

        std::size_t D = static_cast<std::size_t>(std::log2(std::max<std::size_t>(1, n_))) + 3;
        std::vector<Node*> A(D, nullptr);

        std::vector<Node*> roots;
        Node* w = minNode_;
        do {
            roots.push_back(w);
            w = w->right;
        } while (w != minNode_);

        for (Node* start : roots) {
            Node* x = start;
            std::size_t d = static_cast<std::size_t>(x->degree);

            while (d >= A.size()) {
                A.resize(A.size() * 2 + 1, nullptr);
            }

            while (A[d] != nullptr) {
                Node* y = A[d];
                if (less_(y->key, x->key)) {
                    std::swap(x, y);
                }
                linkTrees_(y, x);
                A[d] = nullptr;
                d = static_cast<std::size_t>(x->degree);
                while (d >= A.size()) {
                    A.resize(A.size() * 2 + 1, nullptr);
                }
            }
            A[d] = x;
        }

        minNode_ = nullptr;
        for (Node* x : A) {
            if (!x) continue;
            x->left = x->right = x;
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
        if (!z) return;

        if (!y->mark) {
            y->mark = true;
        } else {
            cut_(y, z);
            cascadingCut_(z);
        }
    }

    void collectAllNodes_(std::vector<Node*>& out) const {
        std::set<Node*> vis;
        std::vector<Node*> stack;

        Node* r = minNode_;
        if (!r) return;

        std::vector<Node*> roots;
        Node* cur = r;
        do {
            roots.push_back(cur);
            cur = cur->right;
        } while (cur != r);

        for (Node* root : roots) {
            stack.push_back(root);
        }

        while (!stack.empty()) {
            Node* x = stack.back();
            stack.pop_back();

            if (!x || vis.count(x)) continue;
            vis.insert(x);
            out.push_back(x);

            if (x->child) {
                Node* c = x->child;
                do {
                    stack.push_back(c);
                    c = c->right;
                } while (c != x->child);
            }
        }
    }

    void copyFrom_(const FibonacciHeap& other) {
        if (!other.minNode_) {
            minNode_ = nullptr;
            n_ = 0;
            return;
        }

        std::vector<Node*> oldAll;
        other.collectAllNodes_(oldAll);

        std::unordered_map<const Node*, Node*> mp;
        mp.reserve(oldAll.size());

        for (Node* old : oldAll) {
            Node* neu = new Node(old->key);
            neu->degree = old->degree;
            neu->mark = old->mark;
            mp[old] = neu;
        }

        for (Node* old : oldAll) {
            Node* neu = mp[old];
            neu->parent = old->parent ? mp[old->parent] : nullptr;
            neu->child  = old->child  ? mp[old->child]  : nullptr;
            neu->left   = old->left   ? mp[old->left]   : neu;
            neu->right  = old->right  ? mp[old->right]  : neu;
        }

        minNode_ = mp.at(other.minNode_);
        n_ = other.n_;
    }
};

