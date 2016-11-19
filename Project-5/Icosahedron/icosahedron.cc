// Draw an Icosahedron
// ECE4893/8893 Project 4
// VIGNESH SRIDHARAN

#include <iostream>
#include <math.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <map>

#include <GL/glut.h>
#include <GL/glext.h>
#include <GL/gl.h>
#include <GL/glu.h>

/*#include <GLUT/glut.h>
#include <OpenGL/glext.h>
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>*/

using namespace std;

#define NFACE 20
#define NVERTEX 12

#define X .525731112119133606 
#define Z .850650808352039932

// These are the 12 vertices for the icosahedron
static GLfloat vdata[NVERTEX][3] = {    
   {-X, 0.0, Z}, {X, 0.0, Z}, {-X, 0.0, -Z}, {X, 0.0, -Z},    
   {0.0, Z, X}, {0.0, Z, -X}, {0.0, -Z, X}, {0.0, -Z, -X},    
   {Z, X, 0.0}, {-Z, X, 0.0}, {Z, -X, 0.0}, {-Z, -X, 0.0} 
};

// These are the 20 faces.  Each of the three entries for each 
// vertex gives the 3 vertices that make the face.
static GLint tindices[NFACE][3] = { 
   {0,4,1}, {0,9,4}, {9,5,4}, {4,5,8}, {4,8,1},    
   {8,10,1}, {8,3,10}, {5,3,8}, {5,2,3}, {2,7,3},    
   {7,10,3}, {7,6,10}, {7,11,6}, {11,0,6}, {0,1,6}, 
   {6,1,10}, {9,0,11}, {9,11,2}, {9,2,5}, {7,2,11} };

int testNumber; // Global variable indicating which test number is desired
int depth; // Global variable indicating what depth is desired
int rotateicos; // Global variable indicating if rotation is desired

float **color;
int j = 0;

float H, W;

void drawTriangle(GLfloat *v1, GLfloat *v2, GLfloat* v3)
{
  glBegin(GL_TRIANGLES);
	glColor3f(color[j][0],color[j][1],color[j][2]);		//Choosing random colours instead of blue alone
	glNormal3fv(v1); glVertex3fv(v1);
	glNormal3fv(v2); glVertex3fv(v2);
	glNormal3fv(v3); glVertex3fv(v3);
  glEnd();
      
  //glLineWidth(2.0);
  glBegin(GL_LINE_LOOP);
  glColor3f(1.0, 1.0, 1.0);	//White lines for the triangle borders
	glNormal3fv(v1); glVertex3fv(v1);
	glNormal3fv(v2); glVertex3fv(v2);
	glNormal3fv(v3); glVertex3fv(v3);
  glEnd();
}

void normalize(GLfloat v[3]) 
{
  GLfloat sz = sqrt(v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);
  
  if(sz == 0.0) {
		return;
  }
  v[0] = v[0] / sz;
  v[1] = v[1] / sz;
  v[2] = v[2] / sz;
}

void subdivide(GLfloat *v1, GLfloat *v2, GLfloat *v3, int depth_stage)
{
   GLfloat v12[3], v23[3], v31[3];		//Midpoints of each triangle side

   if (depth_stage == 0) {
      drawTriangle(v1, v2, v3);
      j++;
      return;
   }

   //Finding midpoints
   for (int i = 0; i < 3; i++) {
      v12[i] = v1[i]+v2[i];
      v23[i] = v2[i]+v3[i];
      v31[i] = v3[i]+v1[i];
   }
   normalize(v12);
   normalize(v23);
   normalize(v31);
   //Recursively drawing the smaller triangles
   subdivide(v1, v12, v31, depth_stage-1);
   subdivide(v2, v23, v12, depth_stage-1);
   subdivide(v3, v31, v23, depth_stage-1);
   subdivide(v12, v23, v31, depth_stage-1);
}
// Test cases.  Fill in your code for each test case
void Test1()
{
	depth = 0;
	rotateicos = 0;
}

void Test2()
{
	depth = 0;
	rotateicos = 1;
}

void Test3()
{
	depth = 1;
	rotateicos = 0;
}

void Test4()
{
	depth = 1;
	rotateicos = 1;
}

void Test5(int input_depth)
{
	depth = input_depth;
	rotateicos = 0;
}

void Test6(int input_depth)
{
	depth = input_depth;
	rotateicos = 1;
}

static GLfloat updateRate = 40.0;

void display(void)
{
  static int pass;
  	
  cout << "Displaying pass " << ++pass << endl;
  // clear all
  glClear(GL_COLOR_BUFFER_BIT);
  glClear(GL_DEPTH_BUFFER_BIT);
  glEnable(GL_LINE_SMOOTH);		//enable anti-aliasing
  //glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
  // Clear the matrix
  glPushMatrix(); 
  glLoadIdentity(); 

  // Set the viewing transformation
  gluLookAt(0.0, 0.0, 45.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);

  glTranslatef(W/2, H/2, 0); 
  glScalef(W/2, H/2, 1.0);
   
  // Try rotating

  if(rotateicos) { 
  	static GLfloat rotY = 1.0;
  	static GLfloat rotX = 1.0;
  	glRotatef(rotX, 1.0, 0.0, 0.0);
  	glRotatef(rotY, 0.0, 1.0, 0.0);
  	rotX+=1;
  	rotY+=1;
   }

   //Draw the icosahedron
   for(int i=0; i < NFACE;i++) {
			subdivide(&vdata[tindices[i][0]][0], &vdata[tindices [i][1]][0], &vdata[tindices[i][2]][0], depth);
   }
   
   glPopMatrix();
   j = 0; 

   glutSwapBuffers(); // If double buffering
}

void init()
{
  //select clearing (background) color
  glClearDepth(1.0);
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LEQUAL);
  glClearColor(0.0, 0.0, 0.0, 0.0);
  glShadeModel(GL_FLAT); 
}


void reshape(int w, int h)
{
  H = h;
  W = w;
  glViewport(0, 0, (GLsizei)w, (GLsizei)h);
  //Using orthogonal matrix, maps the entire graphics on the 2D plane. 
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(0.0, (GLdouble)w, (GLdouble)0.0, h, (GLdouble)w, (GLdouble)-w); 	
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();  
}

void timer(int)
{
  glutPostRedisplay();
  glutTimerFunc(1000.0 / updateRate, timer, 0);
}

int main(int argc, char** argv)
{
  int icos_depth = 0;
  if (argc < 3) {
    if(argc < 2) {
     	std::cout << "Usage: ./icosahedron testnumber depth" << endl;
      exit(1);
    } 
    else {
  		testNumber = atol(argv[1]);
			icos_depth = 0;
    }
  } 
  else {
  	testNumber = atol(argv[1]);
  	icos_depth = atol(argv[2]);
  }

  switch(testNumber) {
		case 1: { 
			Test1(); 
			break;
		}
		case 2: { 
			Test2(); 
			break;
		}
		case 3: { 
			Test3(); 
			break;
		}
		case 4: { 
			Test4(); 
			break;
		}
		case 5: { 
			Test5(icos_depth); 
			break;
		}
		case 6: { 
			Test6(icos_depth); 
			break;
		}
  };

  //Random Coloring
  int num_triangles = NFACE * pow(4, depth);
  color = new float *[num_triangles];
  for(int i = 0; i < num_triangles; i++) {
		color[i] = new float[3];
  }

  for(int k = 0; k < num_triangles; k++) {
   float a = rand();
   float b = rand();
   float c = rand();
   color[k][0] = a / RAND_MAX;
   color[k][1] = b / RAND_MAX;
   color[k][2] = c / RAND_MAX;
  }

  // Initialize glut  and create your window here
  // Set your glut callbacks here
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
  glutInitWindowSize(500, 500);
  glutInitWindowPosition(100, 100);
  glutCreateWindow("Icosahedron");
  init();

  glutDisplayFunc(display);
  glutReshapeFunc(reshape);

  glutTimerFunc(1000.0 / updateRate, timer, 0);

  // Enter the glut main loop here
  glutMainLoop();
  return 0;
}

