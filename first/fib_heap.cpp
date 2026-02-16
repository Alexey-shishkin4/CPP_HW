#include <cassert>
#include <cmath>
#include <iostream>
#include <limits>
#include <random>
#include <set>
#include <vector>


class FibonacciHeap {
  public:
    struct Node {
      int key;
      int degree = 0;
      bool mark = false;

      Node* parent = nullptr;
      Node* child = nullptr;

      Node* left = this;
      Node* right = this;

      explicit Node(int k) : key(k) {}
    };

  FibonacciHeap() = default;

  FibonacciHeap(const FibonacciHeap&) = delete;
  FibonacciHeap& operator=(const FibonacciHeap&) = delete;

  FibonacciHeap(FibonacciHeap&&) = delete;
  FibonacciHeap& operator=(FibonacciHeap&&) = delete;

  ~FibonacciHeap(){
    clear();
  }

  bool isEmpty() const { return minNode == nullptr; }
  size_t size() const { return n; }

  Node* insert(int key) {
    Node* x = new Node(key);
    insertIntoRootList(x);
    n++;
    return x;
  }

  int getMin() const {
    assert(minNode && "getMin() on empty heap");
    return minNode->key;
  }

  int extractMin() {
    assert(minNode && "extractMin() on empty heap");
    Node* z = minNode;

    if (z->child) {
      std::vector<Node*> kids;
      Node* c = z->child;
      do {
        kids.push_back(c);
        c = c->right;
      } while (c != z->child);

      for (Node* x : kids) {
        removeFromList(x);
        x->parent = nullptr;
        x->mark = false;
        insertIntoRootList(x);
      }
      z->child = nullptr;
      z->degree = 0;
    }

    Node* next = z->right;
    removeFromList(z);
    if (next == z) {
      minNode = nullptr;
    } else {
      minNode = next;
      consolidate();
    }

    int res = z->key;
    delete z;
    n--;
    return res;
  }

  void decreaseKey(Node* x, int newKey) {
    assert(x && "decreasekey: null node");
    assert(newKey <= x->key && "decreasekey: newKey must be <= current key");

    x->key = newKey;
    Node* y = x->parent;

    if (y && x->key < y->key) {
      cut(x, y);
      cascadingCut(y);
    }

    if (minNode && x->key < minNode->key) {
      minNode = x;
    }
  }


  void erase(Node* x) {
    decreaseKey(x, std::numeric_limits<int>::min());
    (void)extractMin();
  }

  void clear() {
    if (!minNode) {
      n = 0;
      return;
    }

    std::vector<Node*> all;
    collectAllNodes(all);

    minNode = nullptr;
    n = 0;

    for (Node* p : all) {
      p->left = p->right = p;
      p->parent = p->child = nullptr;
      delete p;
    }
  }

private:
  Node* minNode = nullptr;
  size_t n = 0;

  void insertIntoRootList(Node* x) {
    x->parent = nullptr;
    x->mark = false;

    if (!minNode) {
      minNode = x;
      x->left = x->right = x;
      return;
    }

    x->left = minNode;
    x->right = minNode->right;
    minNode->right->left = x;
    minNode->right = x;

    if (x->key < minNode->key) minNode = x;
  }

  static void removeFromList(Node* y, Node* x) {
    removeFromList(y);
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
    x->degree++;
  }

  static void removeFromList(Node* x) {
    x->left->right = x->right;
    x->right->left = x->left;
    x->left = x->right = x;
  }

  void linkTrees(Node* y, Node* x) {
    removeFromList(y);
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
    x->degree++;
  }


  void consolidate() {
    if (!minNode) return;

    size_t D = static_cast<size_t>(std::log2(std::max<size_t>(1, n))) + 3;
    std::vector<Node*> A(D, nullptr);

    std::vector<Node*> roots;
    Node* w = minNode;
    do {
      roots.push_back(w);
      w = w->right;
    } while (w != minNode);

    for (Node* start : roots) {
      Node* x = start;
      size_t d = static_cast<size_t>(x->degree);

      while (d >= A.size()) A.resize(A.size() * 2 + 1, nullptr);

      while (A[d] != nullptr) {
        Node* y = A[d];
        if (y->key < x->key) std::swap(x, y);
        linkTrees(y, x);
        A[d] = nullptr;
        d = static_cast<size_t>(x->degree);
        while (d >= A.size()) A.resize(A.size() * 2 + 1, nullptr);
      }
      A[d] = x;
    }

    minNode = nullptr;
    for (Node* x : A) {
      if (!x) continue;
      x->left = x->right = x;
      if (!minNode) {
        minNode = x;
      } else {
        x->left = minNode;
        x->right = minNode->right;
        minNode->right->left = x;
        minNode->right = x;
        if (x->key < minNode->key) minNode = x;
      }
    }
  }

  void cut(Node* x, Node* y) {
    assert(x->parent == y);

    if (y->child == x) {
      if (x->right != x) y->child = x->right;
      else y->child = nullptr;
    }
    removeFromList(x);
    y->degree--;

    x->parent = nullptr;
    x->mark = false;

    x->parent = nullptr;
    x->mark = false;
    insertIntoRootList(x);
  }

  void cascadingCut(Node* y) {
    Node* z = y->parent;
    if (!z) return;

    if (!y->mark) {
      y->mark = true;
    } else {
      cut(y, z);
      cascadingCut(z);
    }
  }


  void collectAllNodes(std::vector<Node*>& out) {
    std::set<Node*> vis;

    std::vector<Node*> stack;
    Node* r = minNode;
    if (!r) return;

    std::vector<Node*> roots;
    Node* cur = r;
    do {
      roots.push_back(cur);
      cur = cur->right;
    } while (cur != r);

    for (Node* root : roots) stack.push_back(root);

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
};
