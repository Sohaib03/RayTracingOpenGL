#include <bits/stdc++.h>
#include <GL/glut.h>
#include "1805026_geo.hpp"
#include "1805026_color.hpp"
using namespace std;

struct Light {
    Point position;
    double fall_off;
    Light() {}
    Light(Point p, double f) : position(p), fall_off(f) {}

    void draw() {
        glPushMatrix();
        glTranslated(position.x, position.y, position.z);
        glColor3f(0.4, 0.4, 0);
        // cout << "Drawing light on " << position << endl;
        glutSolidSphere(10, 10, 10);
        glPopMatrix();
    }
};

struct SpotLight {
    Light point_light;
    Vector3D dir;
    double cutoff_angle;

    void draw() {
        point_light.draw();
    }
};

