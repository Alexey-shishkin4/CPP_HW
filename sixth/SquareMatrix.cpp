#include <iostream>
#include <vector>
#include <stdexcept>
#include <algorithm> // std::copy и std::swap


class Row {
    double* row_ptr;

public:
    Row(double* ptr) : row_ptr(ptr) {}

    double& operator[](size_t col) { return row_ptr[col]; }
    const double& operator[](size_t col) const { return row_ptr[col]; }
};


class SquareMatrix {
    size_t size;
    double* data;

public:
    SquareMatrix(size_t n) : size(n), data(new double[n * n]()) {}

    SquareMatrix(SquareMatrix&& other) noexcept : size(other.size), data(other.data) {
        other.size = 0;
        other.data = nullptr;
    }

    SquareMatrix(const std::vector<double>& diag) : size(diag.size()), data(new double[size * size]()) {
        for (size_t i = 0; i < size; i++)
            (*this)[i][i] = diag[i];
    }

    SquareMatrix(const SquareMatrix& other) : size(other.size), data(new double[other.size * other.size]) {
        std::copy(other.data, other.data + size * size, data);
    }

    SquareMatrix& operator=(SquareMatrix other) {
        swap(other);
        return *this;
    }

    SquareMatrix& operator=(SquareMatrix&& other) noexcept {
        if (this != &other) {
            delete[] data;
            size = other.size;
            data = other.data;

            other.size = 0;
            other.data = nullptr;
        }
        return *this;
    }

    ~SquareMatrix() { delete[] data; }

    void swap(SquareMatrix& other) noexcept {
        std::swap(size, other.size);
        std::swap(data, other.data);
    }

    Row operator[](size_t row) {
        if (row >= size)
            throw std::out_of_range("Row index out of bounds");
        return Row(data + row * size);
    }

    const Row operator[](size_t row) const {
        if (row >= size)
            throw std::out_of_range("Row index out of bounds");
        return Row(data + row * size);
    }

    size_t getSize() const { return size; }

    explicit operator double() const {
        double sum = 0.0;
        for (size_t i = 0; i < size * size; ++i)
            sum += data[i];
        return sum;
    }

    bool operator==(const SquareMatrix& other) const {
        if (size != other.size) return false;
        for (size_t i = 0; i < size * size; i++) {
            if (data[i] != other.data[i]) return false;
        }
        return true;
    }

    bool operator!=(const SquareMatrix& other) const {
        return !(*this == other);
    }

    void checkSizeCompatibility(const SquareMatrix& other) const {
        if (size != other.size)
            throw std::invalid_argument("Matrix sizes are incompatible");
    }

    SquareMatrix operator+(const SquareMatrix& other) const {
        checkSizeCompatibility(other);
        SquareMatrix result(size);
        for (size_t i = 0; i < size * size; i++)
            result.data[i] = data[i] + other.data[i];
        return result;
    }

    SquareMatrix& operator+=(const SquareMatrix& other) {
        checkSizeCompatibility(other);
        for (size_t i = 0; i < size * size; ++i)
            data[i] += other.data[i];
        return *this;
    }

    SquareMatrix operator*(const SquareMatrix& other) const {
        checkSizeCompatibility(other);
        SquareMatrix result(size);
        for (size_t i = 0; i < size; ++i) {
            for (size_t j = 0; j < size; ++j) {
                double sum = 0.0;
                for (size_t k = 0; k < size; ++k)
                    sum += (*this)[i][k] * other[k][j];
                result[i][j] = sum;
            }
        }
        return result;
    }

    SquareMatrix& operator*=(const SquareMatrix& other) {
        SquareMatrix temp = (*this) * other;
        swap(temp);
        return *this;
    }

    SquareMatrix operator*(double scalar) const {
        SquareMatrix result(size);
        for (size_t i = 0; i < size * size; ++i)
            result.data[i] = data[i] * scalar;
        return result;
    }

    SquareMatrix& operator*=(double scalar) {
        for (size_t i = 0; i < size * size; ++i)
            data[i] *= scalar;
        return *this;
    }
};


std::ostream& operator<<(std::ostream& os, const SquareMatrix& m) {
    for (size_t i = 0; i < m.getSize(); ++i) {
        for (size_t j = 0; j < m.getSize(); ++j)
            os << m[i][j] << "\t";
        os << "\n";
    }
    return os;
}


// Свободные функции — поддержка операций с vector<double> слева
SquareMatrix operator+(const std::vector<double>& lhs, const SquareMatrix& rhs) {
    return SquareMatrix(lhs) + rhs;
}

SquareMatrix operator+(const SquareMatrix& lhs, const std::vector<double>& rhs) {
    return lhs + SquareMatrix(rhs);
}

SquareMatrix operator*(const std::vector<double>& lhs, const SquareMatrix& rhs) {
    return SquareMatrix(lhs) * rhs;
}

SquareMatrix operator*(const SquareMatrix& lhs, const std::vector<double>& rhs) {
    return lhs * SquareMatrix(rhs);
}



int main() {
    SquareMatrix m1(3);
    SquareMatrix m2({ 1.0, 2.0, 3.0 });

    m1[0][0] = 1;
    m1[1][1] = 1;
    m1[2][2] = 1;

    SquareMatrix m3 = m1 + m2;
    SquareMatrix m4 = m1 * m2;

    std::cout << "m1:\n" << m1;
    std::cout << "m2:\n" << m2;
    std::cout << "m3 = m1 + m2:\n" << m3;
    std::cout << "m4 = m1 * m2:\n" << m4;

    m3 *= 2.0;
    std::cout << "m3 * 2:\n" << m3;

    double total_sum = static_cast<double>(m3);
    std::cout << "Sum of all elements in m3: " << total_sum << "\n";

    bool are_equal = (m1 == m2);
    std::cout << "m1 == m2? " << std::boolalpha << are_equal << "\n";

    // Проверка операций с vector<double>
    SquareMatrix v_add_left = std::vector<double>{ 5.0, 5.0, 5.0 } + m1;
    SquareMatrix v_add_right = m1 + std::vector<double>{ 4.0, 4.0, 4.0 };
    SquareMatrix v_mul_left = std::vector<double>{ 1.0, 2.0, 3.0 } * m1;
    SquareMatrix v_mul_right = m1 * std::vector<double>{ 2.0, 2.0, 2.0 };

    std::cout << "vector + m1:\n" << v_add_left;
    std::cout << "m1 + vector:\n" << v_add_right;
    std::cout << "vector * m1:\n" << v_mul_left;
    std::cout << "m1 * vector:\n" << v_mul_right;

    return 0;
}
