#include <cassert>
#include <iostream>
#include <random>
#include <set>
#include "fib_heap.cpp"

static void test_basic_min_extract() {
    FibonacciHeap h;
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
    FibonacciHeap h;
    auto* x = h.insert(100);
    h.insert(50);
    h.insert(70);

    assert(h.getMin() == 50);
    h.decreaseKey(x, 1);
    assert(h.getMin() == 1);
    assert(h.extractMin() == 1);
}

static void test_against_multiset_random() {
    FibonacciHeap h;
    std::multiset<int> ms;

    std::mt19937 rng(123);
    std::uniform_int_distribution<int> distV(-1000, 1000);

    std::vector<FibonacciHeap::Node*> handles;

    // вставки
    for (int i = 0; i < 2000; ++i) {
        int v = distV(rng);
        handles.push_back(h.insert(v));
        ms.insert(v);
        assert(h.getMin() == *ms.begin());
    }

    // случайные decreaseKey
    std::uniform_int_distribution<int> distI(0, (int)handles.size() - 1);
    for (int t = 0; t < 2000; ++t) {
        int idx = distI(rng);
        auto* p = handles[idx];
        if (!p) continue; // могли удалить

        int curMin = h.getMin();
        (void)curMin;

        int newKey = distV(rng);
        if (newKey > p->key) newKey = p->key; // делаем только уменьшение

        // обновим multiset: удалить старое значение и вставить новое
        auto it = ms.find(p->key);
        assert(it != ms.end());
        ms.erase(it);
        ms.insert(newKey);

        h.decreaseKey(p, newKey);
        assert(h.getMin() == *ms.begin());
    }

    // извлечения
    while (!ms.empty()) {
        int a = h.extractMin();
        int b = *ms.begin();
        ms.erase(ms.begin());
        assert(a == b);
    }
    assert(h.isEmpty());
}

static void test_copy_ctor_same_order() {
    FibonacciHeap h;
    h.insert(10);
    h.insert(3);
    h.insert(7);
    h.insert(1);
    h.insert(5);

    FibonacciHeap c(h);

    while (!h.isEmpty()) {
        int a = h.extractMin();
        int b = c.extractMin();
        assert(a == b);
    }
    assert(c.isEmpty());
}

static void test_copy_ctor_independent_after_ops() {
    FibonacciHeap h;
    auto* p = h.insert(100);
    h.insert(50);
    h.insert(70);

    FibonacciHeap c(h);

    h.decreaseKey(p, 1);

    assert(h.getMin() == 1);
    assert(c.getMin() == 50);
}

static void test_copy_assignment_basic() {
    FibonacciHeap h1;
    h1.insert(9);
    h1.insert(2);
    h1.insert(6);

    FibonacciHeap h2;
    h2.insert(100);
    h2.insert(200);

    h2 = h1;

    while (!h1.isEmpty()) {
        assert(h1.extractMin() == h2.extractMin());
    }
    assert(h2.isEmpty());
}

static void test_self_assignment() {
    FibonacciHeap h;
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
    FibonacciHeap h1;
    auto* p = h1.insert(10);
    h1.insert(20);
    h1.insert(30);

    FibonacciHeap h2;
    h2.insert(999);

    h2 = h1;

    h1.decreaseKey(p, 1);

    assert(h1.getMin() == 1);
    assert(h2.getMin() == 10);
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

    std::cout << "All tests OK\n";
    return 0;
}

