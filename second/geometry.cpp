#include <cmath>

class Point {
public:
    Point() : x_(0.0), y_(0.0) {}
    Point(double x, double y) : x_(x), y_(y) {}

    double x() const { return x_; }
    double y() const { return y_; }

    bool almostEquals(const Point& other, double eps = 1e-9) const {
        return std::fabs(x_ - other.x_) < eps && std::fabs(y_ - other.y_) < eps;
    }

private:
    double x_, y_;
};

// Ax + By + C = 0
class Line {
public:
    Line(double a, double b, double c) : A_(a), B_(b), C_(c) {
        normalize();
    }

    Line(const Point& p1, const Point& p2) {
        // (y2-y1)x + (x1-x2)y + (x2*y1 - x1*y2) = 0
        A_ = p2.y() - p1.y();
        B_ = p1.x() - p2.x();
        C_ = p2.x() * p1.y() - p1.x() * p2.y();
        normalize();
    }

    double A() const { return A_; }
    double B() const { return B_; }
    double C() const { return C_; }

    bool contains(const Point& p, double eps = 1e-9) const {
        return std::fabs(A_ * p.x() + B_ * p.y() + C_) < eps;
    }

    bool intersection(const Line& other, Point& out, double eps = 1e-9) const {
        const double det = A_ * other.B_ - other.A_ * B_;
        if (std::fabs(det) < eps) return false;

        const double x = (B_ * other.C_ - other.B_ * C_) / det;
        const double y = (other.A_ * C_ - A_ * other.C_) / det;
        out = Point(x, y);
        return true;
    }


    Line perpendicularAt(const Point& p) const {
        const double D = A_ * p.y() - B_ * p.x();
        return Line(B_, -A_, D);
    }

private:
    void normalize(double eps = 1e-12) {
        const double len = std::sqrt(A_ * A_ + B_ * B_);
        if (len > eps) {
            A_ /= len;
            B_ /= len;
            C_ /= len;
        }
    }

    double A_, B_, C_;
};

