#include <cassert>
#include <cstddef>
#include <iostream>
#include <stdexcept>
#include <utility>

template <typename Derived, std::size_t Limit>
class InstanceLimiter {
public:
    InstanceLimiter() {
        acquire();
    }

    InstanceLimiter(const InstanceLimiter&) {
        acquire();
    }

    InstanceLimiter(InstanceLimiter&&) {
        acquire();
    }

    ~InstanceLimiter() {
        release();
    }

    InstanceLimiter& operator=(const InstanceLimiter&) = default;
    InstanceLimiter& operator=(InstanceLimiter&&) = default;

    static std::size_t alive() noexcept {
        return count_;
    }

    static constexpr std::size_t limit() noexcept {
        return Limit;
    }

private:
    inline static std::size_t count_ = 0;

    static void acquire() {
        if (count_ >= Limit) {
            throw std::runtime_error("instance limit exceeded");
        }
        ++count_;
    }

    static void release() noexcept {
        if (count_ > 0) {
            --count_;
        }
    }
};

class Connection : public InstanceLimiter<Connection, 2> {
public:
    Connection() = default;
};

class Worker : public InstanceLimiter<Worker, 3> {
public:
    Worker() = default;
};

class UniqueResource : public InstanceLimiter<UniqueResource, 1> {
public:
    UniqueResource() = default;
};

static void test_basic_limit() {
    assert(Connection::alive() == 0);

    Connection a;
    Connection b;
    assert(Connection::alive() == 2);

    bool thrown = false;
    try {
        Connection c;
    } catch (const std::runtime_error&) {
        thrown = true;
    }

    assert(thrown);
    assert(Connection::alive() == 2);
}

static void test_release_after_scope() {
    assert(Connection::alive() == 0);

    {
        Connection a;
        assert(Connection::alive() == 1);
    }

    assert(Connection::alive() == 0);
}

static void test_copy_constructor_counts_instance() {
    assert(Worker::alive() == 0);

    Worker a;
    Worker b;
    assert(Worker::alive() == 2);

    Worker c = a;
    assert(Worker::alive() == 3);

    bool thrown = false;
    try {
        Worker d;
    } catch (const std::runtime_error&) {
        thrown = true;
    }

    assert(thrown);
    assert(Worker::alive() == 3);
}

static void test_move_constructor_counts_instance() {
    assert(UniqueResource::alive() == 0);

    UniqueResource a;
    assert(UniqueResource::alive() == 1);

    bool thrown = false;
    try {
        UniqueResource b(std::move(a));
    } catch (const std::runtime_error&) {
        thrown = true;
    }

    assert(thrown);
    assert(UniqueResource::alive() == 1);
}

static void test_independent_counters() {
    assert(Connection::alive() == 0);
    assert(Worker::alive() == 0);

    Connection c1;
    Worker w1;
    Worker w2;

    assert(Connection::alive() == 1);
    assert(Worker::alive() == 2);
}

int main() {
    test_basic_limit();
    test_release_after_scope();
    test_copy_constructor_counts_instance();
    test_move_constructor_counts_instance();
    test_independent_counters();

    std::cout << "All tests OK\n";
    return 0;
}

