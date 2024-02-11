#include "1805026_light.hpp"
#include "1805026_bitmap_image.hpp"

struct Coeff {
    double a_coeff, d_coeff, s_coeff, r_coeff, shinyness;
};
struct Object {
    Point ref;
    Color col;
    Coeff cf;
    virtual void draw() = 0;
    virtual double intersect(Ray ray, Color &color, int level, Ray &refRay, Point &CP, Point &N) = 0;
};

Ray reflect(Ray& ray, Point& normal, Point& point) {
    Point incident_vector = point - ray.src;
    double dot_product = incident_vector * normal;
    Point reflection_vector = incident_vector - normal * 2 * dot_product;
    Ray reflected_ray(point, reflection_vector);
    return reflected_ray;
}

extern bitmap_image tex_1, tex_2;
extern bool texture_mode;
extern Point pos;

const double EPS = 1e-2;
struct Floor: Object {
    int tile_cnt;
    int t_width;

    Floor() {
        tile_cnt = t_width = 1;
    }

    Floor(int f_width, int t_width) {
        this->t_width = t_width;
        tile_cnt = 100;
        ref = Point(-(tile_cnt/2)*t_width, -(tile_cnt/2)*t_width, 0);
    }

    virtual void draw() {
        for (int i=0; i<tile_cnt; i++) {
            for (int j=0; j<tile_cnt; j++) {
                if ( (i + j) & 1) glColor3f(1, 1, 1);
                else glColor3f(0, 0, 0);

                glBegin(GL_QUADS);

                glVertex3f(ref.x + i*t_width, ref.y + j*t_width, 0);
                glVertex3f(ref.x + (i + 1)*t_width, ref.y + j*t_width, 0);
                glVertex3f(ref.x + (i + 1)*t_width, ref.y + (j+1)*t_width, 0);
                glVertex3f(ref.x + i*t_width, ref.y + (j+1)*t_width, 0);

                glEnd();
            }
        }
    }

    virtual double intersect(Ray ray, Color &color, int level, Ray &refRay, Point &CP, Point &Nrm) {
        Point normal = getNormal(ray);
        // double dotP = normal * ray.dir;
        // if (abs(dotP) < EPS) return -1;
        if(ray.dir.z == 0) return -1;

        double t = -ray.src.z / ray.dir.z;
        if( t < 0) return -1;
        Point p = ray.src + ray.dir * t;

        // if (level ==1) cout << "Here" << endl;
        // if (p.x <= ref.x)  return -1;
        // if (p.x >= abs(ref.x) and p.y <= ref.y and p.y >= abs(ref.y))
        //     return -1;
        if (level == 1) {
            color = getColor(p);
            refRay = ray;
            refRay.src = p;
            refRay.dir.z *= -1;
            CP = p;
            // if (normal * ray.dir > EPS) normal = normal * -1;
            Nrm = normal;
        }
        return t;
    }

    Color getColor(Point pt) {
        int x = (pt.x - ref.x) / t_width;
        int y = (pt.y - ref.y) / t_width;
        double i = (pt.x - ref.x) - x * t_width;
        double j = (pt.y - ref.y) - y * t_width;

        // cout << x << " " << y << endl;
        if (x < 0 or x >= tile_cnt or y < 0 or y >= tile_cnt) return {0, 0, 0};

        if (!texture_mode) {
            if ( (x + y) & 1) return {1, 1, 1};
            else return {0, 0, 0};
        }

        if ( (x + y) & 1) {
            int px = (int)((i / t_width) * tex_2.width()) % tex_2.width();
            int py = (int)((j / t_width) * tex_2.height()) % tex_2.height();
            // cout << " ** " << px << " " << py << endl;
            rgb_t c = tex_2.get_pixel(px, py);
            return {c.red / 255.0, c.green / 255.0, c.blue / 255.0};
            // return {1, 1, 1};
        }
        else {
            int px = (int)((i / t_width) * tex_1.width()) % tex_1.width();
            int py = (int)((j / t_width) * tex_1.height()) % tex_1.height();
            // cout << " ** " << px << " " << py << endl;
            rgb_t c = tex_1.get_pixel(px, py);
            return {c.red / 255.0, c.green / 255.0, c.blue / 255.0};
        }

    }

    Point getNormal(Ray ray) {
        Point n = {0, 0, 1};
        if (n * ray.dir > 0) n = n * -1;
        return n;
    }
};

void drawTriangle(Point a, Point b, Point c, Color col) {
    glColor3f(col.r, col.g, col.b);
    glBegin(GL_TRIANGLES);
        glColor3f(col.r, col.g, col.b); 
        glVertex3f(a.x, a.y, a.z);
        glVertex3f(b.x, b.y, b.z);
        glVertex3f(c.x, c.y, c.z);
    glEnd();
}

struct Triangle: Object {
    Point a, b, c;

    Triangle() {}
    Triangle(Point _a, Point _b, Point _c) {
        a = _a; b = _b; c = _c;
    }

    virtual void draw() {
        drawTriangle(a, b, c, col);
    }

    Vector3D getNormal() {
        auto k = ((b-a) ^ (c-a));
        k.normalize();
        return k;
    }

    double getTwiceArea(Point a, Point b, Point c) {
        Vector3D k = (b-a) ^ (c-a);
        return k.length();
    }

    bool isInside(Point p) {
        return abs( getTwiceArea(a, b, p) + getTwiceArea(a, c, p) + getTwiceArea(b, c, p) - getTwiceArea(a, b, c)) < EPS;
    }

    virtual double intersect(Ray ray, Color &color, int level, Ray& refRay, Point &CP, Point &Nrm) { 
        return 0;
    }

    double intersect(Ray ray, Color &color, int level, Point &p, Point &normal) {
        Point x = ray.src;
        Point d = ray.dir;

        normal = getNormal();
        if (normal * ray.dir > EPS) normal = normal * -1;
        double y = normal * a;
        double dotP = normal * d;
        if (abs(dotP) < EPS) return -1;
        double t = ( y - normal * x ) / (dotP);
        p = x + d * t;

        if (isInside(p)) {
            if (level == 1) {
                color = col;
                normal = getNormal();
                if (normal * ray.dir > EPS) normal = normal * -1;
            }
            return t;
        }

        return -1;
    }
};

struct Sphere: Object {
    double radius;
    Sphere() {}
    Sphere(Point center, double radius) {
        ref = center;
        this->radius = radius;
    }

    vector<vector<Point>> generatePoints(int stack_cnt, int slice_cnt) {
        vector<vector<Point>> pts(stack_cnt + 3, vector<Point>(slice_cnt + 3));
        double h, r;
        for (int i = 0; i <= stack_cnt; i++)
        {
            h = radius * sin(((double)i / (double)stack_cnt) * (pi / 2));
            r = radius * cos(((double)i / (double)stack_cnt) * (pi / 2));
            for (int j = 0; j <= slice_cnt; j++)
            {
                pts[i][j] = {r * cos(((double)j / (double)slice_cnt) * 2 * pi),
                            r * sin(((double)j / (double)slice_cnt) * 2 * pi),
                            pts[i][j].z = h};
            }
        }
        return pts;
    }

    virtual void draw() {
        int stack_cnt = 32;
        int slice_cnt = 32;

        auto points = generatePoints(stack_cnt, slice_cnt);

        for (int i = 0; i < stack_cnt; i++)
        {
            glPushMatrix();
            glTranslatef(ref.x, ref.y, ref.z);
            glColor3f(col.r, col.g, col.b);
            for (int j = 0; j < slice_cnt; j++)
            {
                glBegin(GL_QUADS);
                {
                    glVertex3f(points[i][j].x, points[i][j].y, points[i][j].z);
                    glVertex3f(points[i][j + 1].x, points[i][j + 1].y, points[i][j + 1].z);
                    glVertex3f(points[i + 1][j + 1].x, points[i + 1][j + 1].y, points[i + 1][j + 1].z);
                    glVertex3f(points[i + 1][j].x, points[i + 1][j].y, points[i + 1][j].z);
                    glVertex3f(points[i][j].x, points[i][j].y, -points[i][j].z);
                    glVertex3f(points[i][j + 1].x, points[i][j + 1].y, -points[i][j + 1].z);
                    glVertex3f(points[i + 1][j + 1].x, points[i + 1][j + 1].y, -points[i + 1][j + 1].z);
                    glVertex3f(points[i + 1][j].x, points[i + 1][j].y, -points[i + 1][j].z);
                }
                glEnd();
            }
            glPopMatrix();
        } 
    }
    virtual double intersect(Ray ray, Color &color, int level, Ray &refRay, Point &CP, Point &Nrm) {
        Point x = ray.src;
        Point d = ray.dir;

        x = x - ref;
        double a = d * d, b = 2 * (d * x);
        double c = x * x - radius * radius;
        double disc = b * b - 4 * a * c;
        double t = -1;
        if (disc < EPS)  {
            t = -1;
        }
        else {
            if (abs(a) < EPS) {
                t = -c/b;
                return t;
            }

            double t1 = (- b - sqrt(disc)) / (2 * a);
            double t2 = (- b + sqrt(disc)) / (2 * a);

            if (t2 < t1) swap(t1, t2);
            if (t1 > EPS) t = t1;
            else if (t2 > EPS) t = t2;
        }
        if (level == 1 and t>EPS) {
            color = col;
            Point p = x + d * t;



            Point normal = p;
            p = p + ref;
            normal.normalize();
            refRay = reflect(ray, normal, p);
            CP = p;
            Nrm = normal;
        }
        return t;
    }
};

struct Cube: Object {

    double length;
    bool rest = false;

    Cube() {}
    Cube(Point center, double length) {
        ref = center;
        this->length = length;
        ref = ref + (Point){length/2, length/2, length/2};
        rest = true;
    }

    vector<vector<Point>> faces = {
        { {1, 1, 1}, {1, 1, -1}, {1, -1, -1}, {1, -1, 1} },
        { {1, 1, 1}, {1, -1, 1}, {-1, -1, 1}, {-1, 1, 1} },
        { {1, -1, -1}, {1, 1, -1}, {-1, 1, -1}, {-1, -1, -1} },
        { {-1, 1, 1}, {-1, 1, -1}, {1, 1, -1}, {1, 1, 1} },
        { {-1, -1, -1}, {-1, -1, 1}, {1, -1, 1}, {1, -1, -1} },
        { {-1, 1, 1}, {-1, -1, 1}, {-1, -1, -1}, {-1, 1, -1} }
    };

    virtual void draw() {
        if (!rest) {
            ref = ref + (Point){length/2, length/2, length/2};
            rest = true;
        }
        for (int i=0; i<6; i++) {
            Point a = ref + faces[i][0] * length/2, b = ref + faces[i][1] * length/2,
                c = ref + faces[i][2] * length/2, d = ref + faces[i][3] * length/2;
            // cout << ref << endl;
            // cout  << length  << " " << a << "\n" << b << "\n" << c << "\n" << d << "\n" << endl;
            drawTriangle(a, b, c, col);
            drawTriangle(a, c, d, col);
        }
    }

    virtual double intersect(Ray ray, Color &color, int level, Ray &refRay, Point &CP, Point &Nrm) {
        if (!rest) {
            ref = ref + (Point){length/2, length/2, length/2};
            rest = true;
        }
        Point x = ray.src;
        Point d = ray.dir;
        double t = -1;
        Point p,norm, contact_point, normal;
        // ref = ref + (Point){length/2, length/2, length/2};
        for (int i=0; i<6; i++) { 
            Point a = ref + faces[i][0] * length/2, b = ref + faces[i][1] * length/2,
                c = ref + faces[i][2] * length/2, d = ref + faces[i][3] * length/2;
            double t1 = Triangle(a, b, c).intersect(ray, color, level, p, norm);
            if (t1 > EPS) {
                if (t < EPS or t1 < t) {
                    contact_point = p;
                    normal = norm;
                    t = t1;
                }
            }
            double t2 = Triangle(a, c, d).intersect(ray, color, level, p, norm);
            if (t2 > EPS) {
                if (t < EPS or t2 < t) {
                    contact_point = p;
                    normal = norm;
                    t = t2;
                }
            }
        }
        if (t > EPS and level == 1) {
            color = col;
            refRay = reflect(ray, normal, contact_point);
            CP = contact_point;
            Nrm = normal;
        }
        return t;
    }
};

struct Pyramid : Object{
    double length, height;
    vector<Point> basePoints;
    Point top;
    
    Pyramid() {}
    Pyramid(Point basePoint, double length, double height) {
        ref = basePoint;
        this->length = length;
        this->height = height;
        basePoints = {
            ref, ref + (Point){length, 0, 0}, ref + (Point){length, length, 0} , ref + (Point) {0, length, 0}
        };
        top = {ref.x + length / 2, ref.y + length / 2, ref.z + height};
    }

    void draw() {
        if (basePoints.size() == 0) {
            basePoints = {
                ref, ref + (Point){length, 0, 0}, ref + (Point){length, length, 0} , ref + (Point) {0, length, 0}
            };
            top = {ref.x + length / 2, ref.y + length / 2, ref.z + height};
        }
        drawTriangle(basePoints[0], basePoints[1], basePoints[2], col);
        drawTriangle(basePoints[1], basePoints[2], basePoints[3], col);
        for (int i=0; i<4; i++) {
            drawTriangle(basePoints[i], basePoints[(i+1) % 4], top, col);
        }
    }

    virtual double intersect(Ray ray, Color &color, int level, Ray &refRay, Point &CP, Point &Nrm) {
        Point x = ray.src;
        Point d = ray.dir;

        if (basePoints.empty()) {
            basePoints = {
                ref, ref + (Point){length, 0, 0}, ref + (Point){length, length, 0} , ref + (Point) {0, length, 0}
            };
            top = {ref.x + length / 2, ref.y + length / 2, ref.z + height};
        }
        
        Point p; 
        Point norm;

        Point contact, normal;

        double min_T = -1;
        double t1 = Triangle(basePoints[0], basePoints[1], basePoints[2]).intersect(ray, color, level, p, norm);
        if (t1 > EPS) {
            min_T = t1;
            contact = p;
            normal = norm;
        }
        double t2 = Triangle(basePoints[1], basePoints[2], basePoints[3]).intersect(ray, color, level, p, norm);
        if (t2 > EPS and (min_T < EPS or t2 < min_T)) {
            min_T = t2;
            contact = p;
            normal = norm;
        }
        for (int i=0; i<4; i++) {
            double t3 = Triangle(basePoints[i], basePoints[(i+1) % 4], top).intersect(ray, color, level, p, norm);
            if (t3 > EPS) {
                if (min_T < EPS or t3 < min_T) {
                    min_T = t3;
                    contact = p;
                    normal = norm;
                }
            }
        }

        if (level == 1 and min_T > EPS) {
            color = col;
            refRay = reflect(ray, normal, contact);
            CP = contact;
            Nrm = normal;
        }
        return min_T;
    }
};