#include <bits/stdc++.h>

const double pi = acos(-1.0);

using namespace std;


struct Point {
    double x, y, z, n;

    Point(double _x, double _y, double _z) {
        x = _x; y = _y; z = _z; n = 1.0;
    }

    Point(double _x, double _y, double _z, double _n) {
        x = _x; y = _y; z = _z; n = _n;
    }

    Point(const Point &p) {
        x = p.x; y = p.y; z = p.z; n = p.n;
    }

    Point() {
        x = y = z = 0.0;
        n = 1.0;
    }

    Point operator +(Point o) {
        return {x + o.x, y + o.y, z + o.z, n};
    }

    Point operator -(Point o) {
        return {x - o.x, y - o.y, z - o.z, n};
    }

    Point operator *(double c) {
        return {x*c, y*c, z*c, n};
    }

    Point operator /(double c) {
        return {x/c, y/c, z/c, n};
    }

    double operator *(Point o) {
        return x*o.x + y*o.y + z*o.z;
    }

    double length() {
        return sqrt(x*x + y*y + z*z);
    }

    double normalize() {
        double len = length();
        x /= len;
        y /= len;
        z /= len;
        return len;
    }

    Point operator ^(Point o) {
        return {y*o.z - z*o.y, z*o.x - x*o.z, x*o.y - y*o.x, n};
    }

    friend ostream& operator <<(ostream &out, Point o) {
        out << fixed << setprecision(7);
        out << o.x << " " << o.y << " " << o.z ;
        return out;
    }

    friend istream& operator >>(istream &in, Point &o) {
        in >> o.x >> o.y >> o.z;
        o.n = 1;
        return in;
    }
};

typedef Point Vector3D;

struct Ray {
    Point src;
    Vector3D dir;

    Ray(Point _src, Vector3D _dir) {
        src = _src;
        dir = _dir;
        dir.normalize();
    }
};