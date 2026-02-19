// tests.cpp
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

int main() {
    test_basic_min_extract();
    test_decrease_key_cut();
    test_against_multiset_random();
    std::cout << "All tests OK\n";
    return 0;
}
