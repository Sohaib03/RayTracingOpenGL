#include <bits/stdc++.h>

#ifndef COLOR_HPP
#define COLOR_HPP

struct Color {
    double r, g, b;
    friend std::istream& operator>>(std::istream& in, Color& color) {
        in >> color.r >> color.g >> color.b;
        return in;
    }
    void normalize() {
        r = std::min(r, 1.0);
        g = std::min(g, 1.0);
        b = std::min(b, 1.0);

        r = std::max(r, 0.0);
        g = std::max(g, 0.0);
        b = std::max(b, 0.0);
    }

    // void modify(double a_coeff, double d)
    void modify(double lam, double phong, double a, double d, double s, Color &o, double ref) {
        // cout << o.r << " " << o.g << " " << o.b << endl;
        // cout << lam << " " << phong << " " << d << " " << s << endl;
        r = a * r + d * lam * r + s * phong * r + ref * o.r;
        g = a * g + d * lam * g + s * phong * g + ref * o.g;
        b = a * b + d * lam * b + s * phong * b + ref * o.b;
        // cout << "* " << r << " " << g << " " << b << endl;
    }
};

#endif