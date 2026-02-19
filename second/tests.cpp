#include <cassert>
#include <iostream>
#include "geometry.cpp"

static void test_line_from_points() {
    Point p1(0, 0), p2(1, 1);
    Line l(p1, p2);
    assert(l.contains(p1));
    assert(l.contains(p2));
    assert(l.contains(Point(2, 2)));
}

static void test_line_from_coeffs() {
    Line l(3, 4, -5);
    assert(l.contains(Point(1, 0.5)));
}

static void test_intersection() {
    Line l1(Point(0, 1), Point(1, 0));
    Line l2(Point(0, 1), Point(-1, 0));

    Point p;
    assert(l1.intersection(l2, p));
    assert(p.almostEquals(Point(0, 1)));
}

static void test_parallel() {
    Line l1(-2, 1, -1);
    Line l2(-2, 1, -3);

    Point p;
    assert(!l1.intersection(l2, p));
}


static void test_perpendicular() {
    Line l(1, 1, -2);      // x + y - 2 = 0
    Point p(1, 1);
    assert(l.contains(p));

    Line perp = l.perpendicularAt(p); // должно получиться x - y = 0
    assert(perp.contains(p));
    assert(perp.contains(Point(2, 2)));
    assert(perp.contains(Point(0, 0)));
}

int main() {
    test_line_from_points();
    test_line_from_coeffs();
    test_intersection();
    test_parallel();
    test_perpendicular();
    std::cout << "All tests OK\n";
}

