#include <iostream>

template <typename Num>
struct Point {
    Num x, y, z;
    Point() : x(0.0), y(0.0), z(0.0) {}
    Point(Num x, Num y, Num z) : x(x), y(y), z(z) {}
    
    bool operator<(Point &right) {
        return x < right.x && y < right.y && z < right.z;
    }
    bool operator>(Point &right) {
        return x > right.x && y > right.y && z > right.z;
    }
    
    Point operator+(Point right) {
        return Point(x+right.x, y+right.y, z+right.z);
    }
    Point& operator+=(Point &right) {
        this->x += right.x;
        this->y += right.y;
        this->z += right.z;
        return *this;
    }
    Point operator-(Point right) {
        return Point(x-right.x, y-right.y, z-right.z);
    }
    Point operator*(Point right) {
        return Point(x*right.x, y*right.y, z*right.z);
    }
    Point operator/(Num right) {
        return Point(x/right, y/right, z/right);
    }
    Point operator*(Num right) {
        return Point(x*right, y*right, z*right);
    }
    
    float length() {
        return sqrt(x*x+y*y+z*z);
    }
    
    bool closeto(Point<Num> other) {
        const auto eps = __FLT_EPSILON__;
        return abs(x-other.x) < eps && abs(y-other.y) < eps && abs(z-other.z) < eps;
    }
    
    Num sum() {
        return x + y + z;
    }
    
    Num product() {
        return x * y * z;
    }
};

template <typename Num>
std::ostream& operator<<(std::ostream &os, Point<Num>& point) {
    os << point.x << ", " << point.y << ", " << point.z;
    return os;
}

typedef Point<float> Pointf;
