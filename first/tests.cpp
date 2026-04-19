#include <cassert>
#include <functional>
#include <iostream>
#include <random>
#include <set>
#include <string>
#include <vector>
#include "fib_heap1.hpp"

static void test_basic_min_extract() {
    FibonacciHeap<int> h;
    auto* a = h.insert(10);
    (void)a;
    h.insert(3);
    h.insert(7);

    assert(h.getMin() == 3);
    assert(h.extractMin() == 3);
    assert(h.getMin() == 7);
    assert(h.extractMin() == 7);
    assert(h.extractMin() == 10);
    assert(h.isEmpty());
}

static void test_decrease_key_cut() {
    FibonacciHeap<int> h;
    auto* x = h.insert(100);
    h.insert(50);
    h.insert(70);

    assert(h.getMin() == 50);
    h.decreaseKey(x, 1);
    assert(h.getMin() == 1);
    assert(h.extractMin() == 1);
}

static void test_against_multiset_random() {
    FibonacciHeap<int> h;
    std::multiset<int> ms;

    std::mt19937 rng(123);
    std::uniform_int_distribution<int> distV(-1000, 1000);

    std::vector<FibonacciHeap<int>::Node*> handles;

    for (int i = 0; i < 2000; ++i) {
        int v = distV(rng);
        handles.push_back(h.insert(v));
        ms.insert(v);
        assert(h.getMin() == *ms.begin());
    }

    std::uniform_int_distribution<int> distI(0, static_cast<int>(handles.size()) - 1);
    for (int t = 0; t < 2000; ++t) {
        int idx = distI(rng);
        auto* p = handles[idx];
        if (!p) continue;

        int newKey = distV(rng);
        if (newKey > p->key) newKey = p->key;

        auto it = ms.find(p->key);
        assert(it != ms.end());
        ms.erase(it);
        ms.insert(newKey);

        h.decreaseKey(p, newKey);
        assert(h.getMin() == *ms.begin());
    }

    while (!ms.empty()) {
        int a = h.extractMin();
        int b = *ms.begin();
        ms.erase(ms.begin());
        assert(a == b);
    }
    assert(h.isEmpty());
}

static void test_copy_ctor_same_order() {
    FibonacciHeap<int> h;
    h.insert(10);
    h.insert(3);
    h.insert(7);
    h.insert(1);
    h.insert(5);

    FibonacciHeap<int> c(h);

    while (!h.isEmpty()) {
        int a = h.extractMin();
        int b = c.extractMin();
        assert(a == b);
    }
    assert(c.isEmpty());
}

static void test_copy_ctor_independent_after_ops() {
    FibonacciHeap<int> h;
    auto* p = h.insert(100);
    h.insert(50);
    h.insert(70);

    FibonacciHeap<int> c(h);

    h.decreaseKey(p, 1);

    assert(h.getMin() == 1);
    assert(c.getMin() == 50);
}

static void test_copy_assignment_basic() {
    FibonacciHeap<int> h1;
    h1.insert(9);
    h1.insert(2);
    h1.insert(6);

    FibonacciHeap<int> h2;
    h2.insert(100);
    h2.insert(200);

    h2 = h1;

    while (!h1.isEmpty()) {
        assert(h1.extractMin() == h2.extractMin());
    }
    assert(h2.isEmpty());
}

static void test_self_assignment() {
    FibonacciHeap<int> h;
    h.insert(4);
    h.insert(1);
    h.insert(3);

    h = h;

    assert(h.getMin() == 1);
    assert(h.extractMin() == 1);
    assert(h.extractMin() == 3);
    assert(h.extractMin() == 4);
    assert(h.isEmpty());
}

static void test_assignment_independent_after_original_changes() {
    FibonacciHeap<int> h1;
    auto* p = h1.insert(10);
    h1.insert(20);
    h1.insert(30);

    FibonacciHeap<int> h2;
    h2.insert(999);

    h2 = h1;

    h1.decreaseKey(p, 1);

    assert(h1.getMin() == 1);
    assert(h2.getMin() == 10);
}

static void test_move_ctor_basic() {
    FibonacciHeap<int> h1;
    h1.insert(10);
    h1.insert(3);
    h1.insert(7);

    assert(h1.size() == 3);
    assert(h1.getMin() == 3);

    FibonacciHeap<int> h2(std::move(h1));

    assert(h1.isEmpty());
    assert(h1.size() == 0);

    assert(!h2.isEmpty());
    assert(h2.size() == 3);
    assert(h2.getMin() == 3);

    assert(h2.extractMin() == 3);
    assert(h2.extractMin() == 7);
    assert(h2.extractMin() == 10);
    assert(h2.isEmpty());
}

static void test_move_assign_basic() {
    FibonacciHeap<int> h1;
    h1.insert(5);
    h1.insert(1);
    h1.insert(9);

    FibonacciHeap<int> h2;
    h2.insert(100);
    h2.insert(200);

    assert(h1.size() == 3);
    assert(h2.size() == 2);

    h2 = std::move(h1);

    assert(h1.isEmpty());
    assert(h1.size() == 0);

    assert(!h2.isEmpty());
    assert(h2.size() == 3);
    assert(h2.getMin() == 1);

    assert(h2.extractMin() == 1);
    assert(h2.extractMin() == 5);
    assert(h2.extractMin() == 9);
    assert(h2.isEmpty());
}

static void test_move_self_assign() {
    FibonacciHeap<int> h;
    h.insert(4);
    h.insert(2);

    FibonacciHeap<int>& href = h;
    href = std::move(h);

    assert(!href.isEmpty());
    assert(href.size() == 2);
    assert(href.getMin() == 2);
}

static void test_move_chain() {
    FibonacciHeap<int> h1;
    for (int i = 0; i < 100; ++i) {
        h1.insert(i);
    }

    FibonacciHeap<int> h2(std::move(h1));
    FibonacciHeap<int> h3;
    h3 = std::move(h2);

    assert(h1.isEmpty());
    assert(h2.isEmpty());
    assert(h3.size() == 100);
    assert(h3.getMin() == 0);

    for (int i = 0; i < 100; ++i) {
        int x = h3.extractMin();
        assert(x == i);
    }
    assert(h3.isEmpty());
}

static void test_string_keys() {
    FibonacciHeap<std::string> h;
    h.insert("pear");
    h.insert("apple");
    h.insert("banana");

    assert(h.getMin() == "apple");
    assert(h.extractMin() == "apple");
    assert(h.extractMin() == "banana");
    assert(h.extractMin() == "pear");
    assert(h.isEmpty());
}

static void test_custom_comparator_max_heap_behavior() {
    FibonacciHeap<int, std::greater<int>> h;
    h.insert(10);
    h.insert(3);
    h.insert(7);

    assert(h.getMin() == 10);
    assert(h.extractMin() == 10);
    assert(h.extractMin() == 7);
    assert(h.extractMin() == 3);
    assert(h.isEmpty());
}

static void test_decrease_key_with_custom_comparator() {
    FibonacciHeap<int, std::greater<int>> h;
    auto* p = h.insert(10);
    h.insert(20);
    h.insert(15);

    assert(h.getMin() == 20);

    h.decreaseKey(p, 100);

    assert(h.getMin() == 100);
    assert(h.extractMin() == 100);
}

static void test_range_based_for_sum() {
    FibonacciHeap<int> h;
    h.insert(10);
    h.insert(3);
    h.insert(7);
    h.insert(1);

    int sum = 0;
    int count = 0;
    for (int x : h) {
        sum += x;
        ++count;
    }

    assert(count == 4);
    assert(sum == 21);
}

static void test_range_based_for_const_heap() {
    FibonacciHeap<int> h;
    h.insert(4);
    h.insert(9);
    h.insert(2);

    const FibonacciHeap<int>& ch = h;

    int sum = 0;
    int count = 0;
    for (const int& x : ch) {
        sum += x;
        ++count;
    }

    assert(count == 3);
    assert(sum == 15);
}

static void test_range_based_for_multiset_equivalence() {
    FibonacciHeap<int> h;
    std::multiset<int> expected = {5, 1, 8, 1, 9, 3};

    for (int x : expected) {
        h.insert(x);
    }

    std::multiset<int> actual;
    for (int x : h) {
        actual.insert(x);
    }

    assert(actual == expected);
}

static void test_iterator_after_copy() {
    FibonacciHeap<int> h;
    h.insert(5);
    h.insert(1);
    h.insert(8);

    FibonacciHeap<int> c(h);

    std::multiset<int> vals;
    for (int x : c) {
        vals.insert(x);
    }

    assert(vals.size() == 3);
    assert(vals.count(1) == 1);
    assert(vals.count(5) == 1);
    assert(vals.count(8) == 1);
}

static void test_iterator_after_move() {
    FibonacciHeap<int> h;
    h.insert(11);
    h.insert(6);
    h.insert(14);

    FibonacciHeap<int> moved(std::move(h));

    int count = 0;
    int sum = 0;
    for (int x : moved) {
        ++count;
        sum += x;
    }

    assert(count == 3);
    assert(sum == 31);
}

int main() {
    test_basic_min_extract();
    test_decrease_key_cut();
    test_against_multiset_random();

    test_copy_ctor_same_order();
    test_copy_ctor_independent_after_ops();
    test_copy_assignment_basic();
    test_self_assignment();
    test_assignment_independent_after_original_changes();

    test_move_ctor_basic();
    test_move_assign_basic();
    test_move_self_assign();
    test_move_chain();

    test_string_keys();
    test_custom_comparator_max_heap_behavior();
    test_decrease_key_with_custom_comparator();

    test_range_based_for_sum();
    test_range_based_for_const_heap();
    test_range_based_for_multiset_equivalence();
    test_iterator_after_copy();
    test_iterator_after_move();

    std::cout << "All tests OK\n";
    return 0;
}
