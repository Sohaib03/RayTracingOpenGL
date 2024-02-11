#include <bits/stdc++.h>
#include <GL/glut.h>
#include <windows.h>
#include "1805026_object.hpp"
#include "1805026_bitmap_image.hpp"


// Global variables
Point pos;
Point l;
Point r;
Point u;
Point target;

double speed = 1;

int rec_level, img_h, img_w;
double near_plane, far_plane;
double fov, fovy, fovx;
double aspect_ratio;
double checkerboard_width;
double amb_coeff, diffuse_coeff, ref_coeff;

int no_of_objects;

vector<Sphere> spheres;
vector<Cube> cubes;
vector<Pyramid> pyramids;
vector<Light> lights;
vector<SpotLight> spot_lights;
Floor ground;
bitmap_image image;
bitmap_image tex_1("texture_b.bmp");
bitmap_image tex_2("texture_w.bmp");
int cur_image = 1;
bool texture_mode = false;

vector<Object*> objects;
/* Initialize OpenGL Graphics */
void initGL() {
    // Set "clearing" or background color
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);   // Black and opaque
    glEnable(GL_DEPTH_TEST);   // Enable depth testing for z-culling
}
double ROT_ANG = pi/180;

void rotate3D(Point &vec,Point &axis,double ang){
	// vec = vec*cos(ang)+(vec*axis)*sin(ang);
	vec = vec*cos(ang)+(axis^vec)*sin(ang);
}

Color rayTrace(Ray ray, int level, bool &obstacle) {

    Color c = {0, 0, 0};
    // cout << c.r << endl;
    if (level == 0) return c;
    double t_min = 1000000;
    int nearest_obj = -1;
    string type;

    Point contact_point, normal;
    Ray reflectedRay({0, 0, 0}, {0, 0, 0});
    double tg = ground.intersect(ray, c, 0, reflectedRay, contact_point, normal);
    if (tg > 0 && tg < t_min) {
        t_min = tg;
        nearest_obj = 0;
        type = "ground";
    }

    for (int i=0; i<spheres.size(); i++) {
        double t = spheres[i].intersect(ray, c, 0, reflectedRay, contact_point, normal);
        if (t > 0 && t < t_min) {
            t_min = t;
            nearest_obj = i;
            type = "sphere";
        }
    }
    for (int i=0; i<cubes.size(); i++) {
        double t = cubes[i].intersect(ray, c, 0, reflectedRay, contact_point, normal);
        if (t > 0 && t < t_min) {
            t_min = t;
            nearest_obj = i;
            type = "cube";
        }
    }
    for (int i=0; i<pyramids.size(); i++) {
        double t = pyramids[i].intersect(ray, c, 0, reflectedRay, contact_point, normal);
        if (t > 0 && t < t_min) {
            t_min = t;
            nearest_obj = i;
            type = "pyramid";
        }
    }
    if (obstacle) {
        obstacle = t_min < 100000;
        // cout << "* " << c.r << endl;
        return c;
    }

    if (t_min > 100000) return c;
    
    Coeff cf;
    if (nearest_obj != -1)  {
        if (type == "ground") {
            ground.intersect(ray, c, 1, reflectedRay, contact_point, normal);
            cf = ground.cf;
        }
        if (type == "sphere") {
            spheres[nearest_obj].intersect(ray, c, 1, reflectedRay, contact_point, normal);
            cf = spheres[nearest_obj].cf;
        }
        if (type == "cube") {
            cubes[nearest_obj].intersect(ray, c, 1, reflectedRay, contact_point, normal);
            cf = cubes[nearest_obj].cf;
        }
        if (type == "pyramid") {
            pyramids[nearest_obj].intersect(ray, c, 1, reflectedRay, contact_point, normal);
            cf = pyramids[nearest_obj].cf;
        }
    }
    // cout << c.r << endl;
    reflectedRay.src = reflectedRay.src + reflectedRay.dir * 0.1;
    Ray lightray({0, 0, 0}, {0, 0, 0});
    double lam = 0, phong = 0;
    for (auto light: lights) {
        lightray.src = contact_point;
        lightray.dir = light.position - contact_point;
       
        lightray.dir.normalize();
        lightray.src = lightray.src + lightray.dir * 0.1;

        bool obs = true;
        rayTrace(lightray, 1, obs);
        if (obs) continue;

        double fall_off = light.fall_off;
        double dist = (light.position - contact_point).length();
        double sf = exp(-dist * dist * fall_off);

        normal = {0, 0, 1};

        lam += lightray.dir * normal * sf;
        reflectedRay.dir.normalize();
        phong += pow(abs(reflectedRay.dir * lightray.dir) + EPS, cf.shinyness) * sf;
    }

    for (auto light : spot_lights) {
        lightray.src = contact_point;
        lightray.dir = light.point_light.position - contact_point;
       
        lightray.dir.normalize();
        lightray.src = lightray.src + lightray.dir * 0.1;

        bool obs = true;
        rayTrace(lightray, 1, obs);
        if (obs) continue;

        Point look = light.point_light.position - light.dir;
        look.normalize();
        double angle = acos(look * lightray.dir) * 180.0 / pi;
        if (angle > light.cutoff_angle) continue;

        double fall_off = light.point_light.fall_off;
        double dist = (light.point_light.position - contact_point).length();
        double sf = exp(-dist * dist * fall_off);

        lam += lightray.dir * normal * sf;
        reflectedRay.dir.normalize();
        phong += pow(abs(reflectedRay.dir * lightray.dir) + EPS, cf.shinyness) * sf;
    }

    bool f = false;
    Color reflectedColor = rayTrace(reflectedRay, level-1, f);
    c.modify(lam, phong, cf.a_coeff, cf.d_coeff, cf.s_coeff , reflectedColor ,cf.r_coeff);
    return c;
}

void capture() {

    for (int i=0; i<img_w; i++) {
        for (int j=0; j<img_h; j++) {
            image.set_pixel(i, j, 0, 0, 0);
        }
    }

    double height = 2 * tan(fovy * (pi / 360)) * near_plane;
    double width = 2 * tan(fovx * (pi / 360)) * near_plane;
    Point screen_mid = pos + l * near_plane;
    double du = width / (img_w * 1.0);
    double dv = height / (img_h * 1.0);
    Point top_left = screen_mid + (u * (height / 2)) - (r * (width / 2));
    int nearest_obj = -1;

    // Point pointBuffer[img_w][img_h];

    for (int i=0; i<img_h; i++) {
        for (int j=0; j<img_w; j++) {
            Point pixel = top_left + (r * (j * du)) - (u * (i * dv));
            Ray ray = Ray(pixel, pixel - pos);
            bool obs = false;
            Color c = rayTrace(ray, rec_level, obs);
            c.normalize();
            image.set_pixel(j, i, 255*c.r, 255*c.g, 255*c.b);
        }
    }
    cout << "Saving" << endl;
    image.save_image("out" + to_string(cur_image++) + ".bmp");
    cout << "Done" << endl;
}



void keyboardListener(unsigned char key, int xx,int yy){
    double rate = 0.01;
	switch(key){
        case '0':
            capture();
            break;
		case '1':
			r.x = r.x*cos(rate)+l.x*sin(rate);
			r.y = r.y*cos(rate)+l.y*sin(rate);
			r.z = r.z*cos(rate)+l.z*sin(rate);

			l.x = l.x*cos(rate)-r.x*sin(rate);
			l.y = l.y*cos(rate)-r.y*sin(rate);
			l.z = l.z*cos(rate)-r.z*sin(rate);
			break;

        case '2':
			r.x = r.x*cos(-rate)+l.x*sin(-rate);
			r.y = r.y*cos(-rate)+l.y*sin(-rate);
			r.z = r.z*cos(-rate)+l.z*sin(-rate);

			l.x = l.x*cos(-rate)-r.x*sin(-rate);
			l.y = l.y*cos(-rate)-r.y*sin(-rate);
			l.z = l.z*cos(-rate)-r.z*sin(-rate);
			break;

        case '3':
			l.x = l.x*cos(rate)+u.x*sin(rate);
			l.y = l.y*cos(rate)+u.y*sin(rate);
			l.z = l.z*cos(rate)+u.z*sin(rate);

			u.x = u.x*cos(rate)-l.x*sin(rate);
			u.y = u.y*cos(rate)-l.y*sin(rate);
			u.z = u.z*cos(rate)-l.z*sin(rate);
			break;

        case '4':
			l.x = l.x*cos(-rate)+u.x*sin(-rate);
			l.y = l.y*cos(-rate)+u.y*sin(-rate);
			l.z = l.z*cos(-rate)+u.z*sin(-rate);

			u.x = u.x*cos(-rate)-l.x*sin(-rate);
			u.y = u.y*cos(-rate)-l.y*sin(-rate);
			u.z = u.z*cos(-rate)-l.z*sin(-rate);
			break;

        case '5':
			u.x = u.x*cos(rate)+r.x*sin(rate);
			u.y = u.y*cos(rate)+r.y*sin(rate);
			u.z = u.z*cos(rate)+r.z*sin(rate);

			r.x = r.x*cos(rate)-u.x*sin(rate);
			r.y = r.y*cos(rate)-u.y*sin(rate);
			r.z = r.z*cos(rate)-u.z*sin(rate);
			break;

        case '6':
			u.x = u.x*cos(-rate)+r.x*sin(-rate);
			u.y = u.y*cos(-rate)+r.y*sin(-rate);
			u.z = u.z*cos(-rate)+r.z*sin(-rate);

			r.x = r.x*cos(-rate)-u.x*sin(-rate);
			r.y = r.y*cos(-rate)-u.y*sin(-rate);
			r.z = r.z*cos(-rate)-u.z*sin(-rate);
			break;
        case 'a':
            rotate3D(r,u,-ROT_ANG);
		    rotate3D(l,u,-ROT_ANG);
            break;
        case 'd':
            rotate3D(r,u,ROT_ANG);
		    rotate3D(l,u,ROT_ANG);
            break;
		case 'w':
			pos.y++;
			break;
		case 's':
            pos.y--;
			break;
        case ' ':
            texture_mode = !texture_mode;
            cout << "Texture mode :" << texture_mode << endl;
            break;

		default:
			break;
	}
	glutPostRedisplay();
}



void specialKeyListener(int key, int x,int y)
{
	switch(key){
		case GLUT_KEY_UP:		//down arrow key
            pos.x += speed * l.x;
            pos.y += speed * l.y;
            target.x += speed * l.x;
            target.y += speed * l.y;
			break;
		case GLUT_KEY_DOWN:		// up arrow key
            pos.x -= speed * l.x;
            pos.y -= speed * l.y;
            target.x -= speed * target.x;
            target.y -= speed * target.y;
			break;

		case GLUT_KEY_RIGHT:
            pos = pos + r * speed;
            target = target + r * speed;
			break;
		case GLUT_KEY_LEFT :
            pos = pos - r * speed;
            target = target - r * speed;
			break;

		case GLUT_KEY_PAGE_UP:
            pos = pos + u * speed;
            target = target + u * speed;
			break;
		case GLUT_KEY_PAGE_DOWN:
            pos = pos - u * speed;
            target = target + u * speed;
			break;

		case GLUT_KEY_INSERT:
			break;

		case GLUT_KEY_HOME:
			break;
		case GLUT_KEY_END:
			break;

		default:
			break;
	}
	glutPostRedisplay();
}

void drawAxes() {
    glLineWidth(3);
    glBegin(GL_LINES);
        glColor3f(1,0,0);   // Red
        // X axis
        glVertex3f(0,0,0);
        glVertex3f(1,0,0);

        glColor3f(0,1,0);   // Green
        // Y axis
        glVertex3f(0,0,0);
        glVertex3f(0,1,0);

        glColor3f(0,0,1);   // Blue
        // Z axis
        glVertex3f(0,0,0);
        glVertex3f(0,0,1);
    glEnd();
}

void loadData() {
    ifstream fin("input.txt");

    fin >> near_plane >> far_plane;
    fin >> fov;
    cout << near_plane << " " << far_plane << " " << fov << endl;
    fovy = fov;
    fin >> aspect_ratio;
    fovx = aspect_ratio * fovy;
    fin >> rec_level >> img_h;
    img_w = img_h;

    fin >> checkerboard_width;
    ground = Floor(50, checkerboard_width);
    fin >> ground.cf.a_coeff >> ground.cf.d_coeff >> ground.cf.r_coeff;
    cout << "Done ground" << endl;

    fin >> no_of_objects;

    cout << "# " << no_of_objects << endl;

    for (int i=0; i<no_of_objects; i++) {
        string type;
        fin >> type;
        cout << "Type : " << type << endl;
        if (type == "sphere") {
            Sphere sphere;
            fin >> sphere.ref;
            fin >> sphere.radius;
            fin >> sphere.col;
            fin >> sphere.cf.a_coeff >> sphere.cf.d_coeff >> sphere.cf.s_coeff >> sphere.cf.r_coeff;
            fin >> sphere.cf.shinyness;

            spheres.push_back(sphere);
            objects.push_back(&spheres.back());
        }
        if (type == "cube") {
            Cube cube;
            fin >> cube.ref;
            fin >> cube.length;
            fin >> cube.col;
            fin >> cube.cf.a_coeff >> cube.cf.d_coeff >> cube.cf.s_coeff >> cube.cf.r_coeff;
            fin >> cube.cf.shinyness;

            cubes.push_back(cube);
            objects.push_back(&cubes.back());
        }
        if (type == "pyramid") {
            Pyramid pyramid;
            fin >> pyramid.ref;
            fin >> pyramid.length >> pyramid.height;
            fin >> pyramid.col;
            fin >> pyramid.cf.a_coeff >> pyramid.cf.d_coeff >> pyramid.cf.s_coeff >> pyramid.cf.r_coeff;
            fin >> pyramid.cf.shinyness;

            pyramids.push_back(pyramid);
            objects.push_back(&pyramids.back());
        }
        cout << "Done " << type << endl;
    }

    int no_of_light_sources;
    fin >> no_of_light_sources;
    while(no_of_light_sources--) {
        Point p;
        double f;
        fin >> p >> f;
        Light light(p, f);
        lights.push_back(light);
    }

    cout << "Done light" << endl;

    int no_of_spot_lights;
    fin >> no_of_spot_lights;
    while(no_of_spot_lights--) {
        Point p;
        double f;
        Vector3D dir;
        double cutoff_angle;
        fin >> p >> f >> dir >> cutoff_angle;
        SpotLight spot_light;
        spot_light.point_light = Light(p, f);
        spot_light.dir = dir - p;
        spot_light.cutoff_angle = cutoff_angle;
        spot_lights.push_back(spot_light);
    }
    cout << "Done input" << endl;
    fin.close();
}
void display() {
    // glClear(GL_COLOR_BUFFER_BIT);            // Clear the color buffer (background)
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);             // To operate on Model-View matrix
    glLoadIdentity();                       // Reset the model-view matrix

    // default arguments of gluLookAt
    // gluLookAt(0,0,0, 0,0,-100, 0,1,0);
    // control viewing (or camera)
    gluLookAt(pos.x,pos.y,pos.z,
              pos.x+l.x,pos.y+l.y,pos.z+l.z,
              u.x,u.y,u.z);
    drawAxes();
    ground.draw();
    for (auto s: spheres) s.draw();
    for (auto c: cubes) c.draw();
    for (auto p: pyramids) p.draw();
    for (auto l: lights) l.draw();
    for (auto sl: spot_lights) sl.draw();
    glutSwapBuffers();  // Render now

}

/* Handler for window re-size event. Called back when the window first appears and
   whenever the window is re-sized with its new width and height */
void reshape(GLsizei width, GLsizei height) {  // GLsizei for non-negative integer
    if (height == 0) height = 1;                // To prevent divide by 0
    GLfloat aspect = (GLfloat)width / (GLfloat)height;

    glViewport(0, 0, width, height);

    glMatrixMode(GL_PROJECTION);  // To operate on the Projection matrix
    glLoadIdentity();             // Reset the projection matrix
    gluPerspective(fov, aspect, near_plane, far_plane);
}

int main(int argc, char** argv) {
    loadData();
    image = bitmap_image(img_w, img_h);
    pos = {0, -160, 60};

    // target.x=target.y=target.z=0;
    l.x=0;l.y=1;l.z=0;
    u.x=0;u.y=0;u.z=1;
    r.x=1;r.y=0;r.z=0;
    // cameraPlacement();
    
    cout << tex_1.width() << " " << tex_1.height() << endl;
    cout << tex_2.width() << " " << tex_2.height() << endl; 
    cout << tex_1.get_pixel(0, 0).red << endl;

    glutInit(&argc, argv);                  // Initialize GLUT
    glutInitWindowSize(640, 640);           // Set the window's initial width & height
    glutInitWindowPosition(50, 50);         // Position the window's initial top-left corner
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGB);	//Depth, Double buffer, RGB color
    glutCreateWindow("OpenGL 3D Drawing 2");          // Create a window with the given title
    glutDisplayFunc(display);               // Register display callback handler for window re-paint
    glutReshapeFunc(reshape);               // Register callback handler for window re-shape

	glutKeyboardFunc(keyboardListener);
	glutSpecialFunc(specialKeyListener);

    initGL();                               // Our own OpenGL initialization
    glutMainLoop();                         // Enter the event-processing loop

    tex_1.clear();
    tex_2.clear();

    spheres.clear();
    cubes.clear();
    pyramids.clear();
    lights.clear();
    spot_lights.clear();
    objects.clear();

    return 0;
}