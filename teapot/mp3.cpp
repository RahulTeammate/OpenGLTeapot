//  
// Example code for OpenGL programming
//
#include "lodepng.h"
#include <GL/glew.h> //This MUST come first. Or else the compiler freaks out.
#include <stdio.h>
#include <stdlib.h>
#include <GL/glut.h>
#include <ctime>
#include <string.h>
#include <math.h>

int nFPS = 20;
float fRotateAngle = 0.f;
clock_t startClock=0,curClock;
long long int prevF=0,curF=0;
int dipMode=1;

typedef struct verticesMakeMeCry
{
	float x;
	float y;
	float z;
} vertex;

typedef struct facesToo
{
	int idxOf1st;
	int idxOf2nd;
	int idxOf3rd;
} face;


face * faceList = NULL;
vertex * vertexList = NULL;

int numOfVertices = 0;
int numOfFaces = 0;

vertex * faceNormalList = NULL;
vertex * vertexNormalList = NULL;

GLuint environ_tex;
GLuint mappy_tex;

int normal = 0;

void program_end();
void program_end()
{
	free(faceList);
	free(vertexList);
	free(faceNormalList);
	free(vertexNormalList);
	printf("demonstration finished.\n");
}

//----------------------------------------------------------------------------------------------------------------------------------------------------
//----------------------------ABOVE IS INCLUDES AND STRUCTS. BELOW IS THE CODE.---------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------------------------------------------------------

void init(void)
{
	// init your data, setup OpenGL environment here
	glClearColor(0.9,0.9,0.9,1.0); // clear color is gray		
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // uncomment this function if you only want to draw wireframe model
					// GL_POINT / GL_LINE / GL_FILL (default)
	glPointSize(4.0);
	
	//My C parser algorithm used from CS241
	FILE * nailFile = fopen("teapot_0.obj","r");

	size_t bufferSize = 0;
	char * buffer = NULL;
	ssize_t checker = 0;

	while (1) //This loop goes through obj file and gathers how many faces and vertices we have.
	{
		checker = getline(&buffer,&bufferSize,nailFile);
		if (checker == -1)
			break;

		if (buffer[0] == 'f')
			numOfFaces++;
		else if (buffer[0] == 'v')
			numOfVertices++;
	}
	fclose(nailFile);

	bufferSize = 0;
	free(buffer);
	buffer = NULL;
	checker = 0;

	printf("number of vertices in this model: %d\n",numOfVertices);
	printf("number of faces in this model: %d\n",numOfFaces);

	vertexList = (vertex*)malloc(sizeof(vertex)*numOfVertices);
	faceList = (face*)malloc(sizeof(vertex)*numOfFaces);
	
	vertexNormalList = (vertex*)malloc(sizeof(vertex)*numOfVertices);
	faceNormalList = (vertex*)malloc(sizeof(vertex)*numOfFaces);
	
	FILE * fileFiller = fopen("teapot_0.obj","r");

	/**
	
	Strtok Test
	
	char * test = (char*)malloc(sizeof("CS241 CS241 \n\0"));
	strcpy(test, "CS241 CS241 \n\0");
	printf("%s\n", strtok(test, " ")); //prints CS241\n
	printf("%s\n", strtok(NULL, " ")); //prints CS241\n
	printf("%s\n", strtok(NULL, " ")); //prints \n\n
	printf("%p\n", strtok(NULL, " ")); //prints (null)\n. If %s was used, segfault.
	printf("end of tok test\n");
	free(test);
	*/
	
	int vertexIdx = 0;
	int faceIdx = 0;	
	
	// typical line of obj file: f 1 55 23\n\0 (Used as e.g in strtok notes in the next lines)

	while (1) //This loop goes through the obj file and fills up both the vertex and face list.
	{

		checker = getline(&buffer,&bufferSize,fileFiller);
		if (checker == -1)
			break;

		//Middleman for safety reasons
		char * middleMan = (char*)malloc(strlen(buffer)+1);
		strcpy(middleMan,buffer);
		
		strtok(middleMan," "); //returns and gets rid of f
		
		if (buffer[0] == 'v') //parse a line 
		{
			vertexList[vertexIdx].x = strtof(strtok(NULL, " "), NULL);
			vertexList[vertexIdx].y = strtof(strtok(NULL, " "), NULL);
			vertexList[vertexIdx].z = strtof(strtok(NULL, " "), NULL);
			
			//printf("v %f %f %f\n", vertexList[vertexIdx].x, vertexList[vertexIdx].y, vertexList[vertexIdx].z);
			
			vertexIdx++;
		}
		else if (buffer[0] == 'f')
		{
			faceList[faceIdx].idxOf1st = atoi(strtok(NULL, " ")) - 1; //returns and gets rid of 1 (now 0 due to vertex index
										       //starting at 0, so we -1 to accomidate that)
			faceList[faceIdx].idxOf2nd = atoi(strtok(NULL, " ")) - 1; //returns and gets rid of 55 (now 54).
			faceList[faceIdx].idxOf3rd = atoi(strtok(NULL, " ")) - 1; //returns and gets rid of 23 (now 22).
			
			//error - you used buffer instead of NULL in the strtoks here. Using buffer instead of NULL will reset the tokening
			//i.e. return and get rid of f over and over.
			
			//printf("f %d %d %d\n", faceList[faceIdx].idxOf1st, faceList[faceIdx].idxOf2nd, faceList[faceIdx].idxOf3rd);
			
			faceIdx++;
		}
		free(middleMan);
	}
	fclose(fileFiller);
	
	free(buffer);
	buffer = NULL;
	
	
	//These next loops should be here, not in the display function, because they give constant values for normals
	//no matter what the display view is.
	
	
	//First loop iterates through all faces and finds their normals (and normalizes them too). 
	//They are put into a list of face normals: normalFaceList.
	//As expected, there are as many faces as there are face normals, so each face at a certain index has
	//their face normal in the same index (but in different lists).
	for (int i = 0; i < numOfFaces; i++)
	{
		int indexOfV1 = faceList[i].idxOf1st;
		int indexOfV2 = faceList[i].idxOf2nd;
		int indexOfV3 = faceList[i].idxOf3rd;
	
		//a cross b = (a2b3-a3b2,a3b1-a1b3, a1b2-a2b1). 
		//a is vertex at index V2 - vertex at index V1. b is vertex at index V3 - vertex at index V1
		vertex a;
		vertex b;
	
		a.x = vertexList[indexOfV2].x - vertexList[indexOfV1].x;
		a.y = vertexList[indexOfV2].y - vertexList[indexOfV1].y;
		a.z = vertexList[indexOfV2].z - vertexList[indexOfV1].z;
	
		b.x = vertexList[indexOfV3].x - vertexList[indexOfV1].x;
		b.y = vertexList[indexOfV3].y - vertexList[indexOfV1].y;
		b.z = vertexList[indexOfV3].z - vertexList[indexOfV1].z; //by the way, vertex behaves as vector for the normal lists.
	
		faceNormalList[i].x = a.y * b.z - a.z * b.y;
		faceNormalList[i].y = a.z * b.x - a.x * b.z;
		faceNormalList[i].z = a.x * b.y - a.y * b.x; //find face normal
		
		float normalizer = sqrt( faceNormalList[i].x * faceNormalList[i].x +
					 faceNormalList[i].y * faceNormalList[i].y +
					 faceNormalList[i].z * faceNormalList[i].z );
		
		faceNormalList[i].x /= normalizer;
		faceNormalList[i].y /= normalizer; //normalize the face normal
		faceNormalList[i].z /= normalizer;
		
	}
	//Second loop iterates through all the vertices. It takes the index of a vertex and searches
	//for it in the faceList. If it is found for a face, it is an index of that face and that face's
	//normal is added to the normal of the vertex of interest. After traversing the entire face list, we
	//normalize the normal of the vertex.
	for (int i = 0; i < numOfVertices; i++) //i = index of a vertex (and its to-be normalized normal)
	{
		vertexNormalList[i].x = 0;
		vertexNormalList[i].y = 0;
		vertexNormalList[i].z = 0;
		
		for (int j = 0; j < numOfFaces; j++) //j = index of a face (and its normalized normal)
		{
			if (faceList[j].idxOf1st == i || faceList[j].idxOf2nd == i || faceList[j].idxOf3rd == i )
			{
				//This means that the vertex index is a vertex of a face. So we add that face's normal to
				//the vertex normal.
				
				vertexNormalList[i].x += faceNormalList[j].x; //AHHHHHHHHHHH!!! Mixed up j with i.
				vertexNormalList[i].y += faceNormalList[j].y;
				vertexNormalList[i].z += faceNormalList[j].z;
				
			}
		}
		
		float normalizer = sqrt( vertexNormalList[i].x * vertexNormalList[i].x +
					 vertexNormalList[i].y * vertexNormalList[i].y +
					 vertexNormalList[i].z * vertexNormalList[i].z );
			
		vertexNormalList[i].x /= normalizer;
		vertexNormalList[i].y /= normalizer; //normalize the face normal
		vertexNormalList[i].z /= normalizer;
	}
	
	//all you need for z-buffer (make sure depth buffer line is there in display func)
	glEnable(GL_DEPTH_TEST);
	
	//For lighting! From lecture Lighting, slide 12 AND http://www.glprogramming.com/red/chapter05.html ------------------------------------------
	GLfloat mat_specular[] = { 1.0, 1.0, 1.0, 1.0 };
	GLfloat mat_shininess[] = { 50.0 };
	GLfloat mat_diffuse[] = {0.8,0.8,0.8,1.0};
	GLfloat mat_ambient[] = {0.2,0.2,0.2,1.0};
	
	GLfloat light_position[] = { 1.0, 1.0, 1.0, 0.0 };
	GLfloat light_white_color[] = {1.0,1.0,1.0,1.0};
	
	glClearColor (1.0, 1.0, 1.0, 1.0);
	glShadeModel (GL_SMOOTH);

	glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
	glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
	glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
	
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);
	glLightfv(GL_LIGHT0, GL_SPECULAR, light_white_color);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light_white_color);
	glLightfv(GL_LIGHT0, GL_AMBIENT, light_white_color);

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	
	//Texture Mapping with lodepng(http://lodev.org/lodepng/) because I hate SOIL.
	
	unsigned char * image;
	unsigned width;
	unsigned height;
	//From Discussion Section 10, Page 3.
	lodepng_decode32_file(&image, &width, &height, "marble_madness.png");
	glGenTextures(1, &mappy_tex);
	
	glActiveTexture(GL_TEXTURE0);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, mappy_tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_DECAL);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, width, height, 0,GL_RGBA, GL_UNSIGNED_BYTE, image);//RGBA since we used decode32, not decode24.
	
	free(image);
	
	lodepng_decode32_file(&image, &width, &height, "campus_probe.png");
	glGenTextures(1, &environ_tex);
	
	glActiveTextureARB(GL_TEXTURE1_ARB);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, environ_tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexGeni(GL_S,GL_TEXTURE_GEN_MODE,GL_SPHERE_MAP);
	glTexGeni(GL_T,GL_TEXTURE_GEN_MODE,GL_SPHERE_MAP);
	glEnable(GL_TEXTURE_GEN_S); //Automatic Texgen for Sphere Mapping. Don't need to manually append it to the vertice construction.
	glEnable(GL_TEXTURE_GEN_T);
	glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, width, height, 0,GL_RGBA, GL_UNSIGNED_BYTE, image);

	free(image);
	
}

void display(void)
{
	if(dipMode==1)
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}else{
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}
	

	curF++;
	// put your OpenGL display commands here
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// reset OpenGL transformation matrix
	//The following line creates perspective distortion.
	glMatrixMode(GL_PROJECTION); //you could also use gluPerspective instead of this line and glFrustum.
	//optional to put glFrustum here.
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity(); // reset transformation matrix to identity

	// setup look at transformation so that 
	// eye is at : (0,0,5)
	// look at center is at : (0,0,0)
	// up direction is +y axis
	gluLookAt(0.f,0.f,5.f,0.f,0.f,0.f,0.f,1.f,0.f);
	glRotatef(fRotateAngle,0.f,1.f,0.f); //this rotates the teapot, not the camera.

	glColor3f(1.0,0.0,0.0); // set current color to red
	glTranslatef(0,-2,0);
  	//glbegin and end should go here
	
	//For debugging where the normals are.
	if (normal == 1)
	{
		glBegin(GL_LINES);

		for (int i = 0; i < numOfFaces; i++)
		{
			int indexOfV1 = faceList[i].idxOf1st;
			int indexOfV2 = faceList[i].idxOf2nd;
			int indexOfV3 = faceList[i].idxOf3rd;
			
			glVertex3f( vertexNormalList[indexOfV1].x + vertexList[indexOfV1].x, 
					vertexNormalList[indexOfV1].y + vertexList[indexOfV1].y,
					vertexNormalList[indexOfV1].z + vertexList[indexOfV1].z  );	
			glVertex3f( vertexList[indexOfV1].x, vertexList[indexOfV1].y, vertexList[indexOfV1].z );
		
			glVertex3f( vertexNormalList[indexOfV2].x + vertexList[indexOfV2].x, 
					vertexNormalList[indexOfV2].y + vertexList[indexOfV2].y,
					vertexNormalList[indexOfV2].z + vertexList[indexOfV2].z  );	
			glVertex3f( vertexList[indexOfV2].x, vertexList[indexOfV2].y, vertexList[indexOfV2].z );
			
			glVertex3f( vertexNormalList[indexOfV3].x + vertexList[indexOfV3].x, 
					vertexNormalList[indexOfV3].y + vertexList[indexOfV3].y,
					vertexNormalList[indexOfV3].z + vertexList[indexOfV3].z  );	
			glVertex3f( vertexList[indexOfV3].x, vertexList[indexOfV3].y, vertexList[indexOfV3].z );
		}
		glEnd();
	}
	//Texture Mapping Here----------------------------------------
	
	
	glBegin(GL_TRIANGLES);
		//This loop iterates through all the faces. From there, we can get the vertices we need to draw and their normal!
		for (int i = 0; i < numOfFaces; i++)
		{
			int indexOfV1 = faceList[i].idxOf1st;
			int indexOfV2 = faceList[i].idxOf2nd;
			int indexOfV3 = faceList[i].idxOf3rd;
			
			//Rectillinear Texture Mapping.
			//Idea from Lecture Texture Mapping 2, Slide 19
			glTexCoord2f(vertexList[indexOfV1].x,vertexList[indexOfV1].y);
			glNormal3f( vertexNormalList[indexOfV1].x, vertexNormalList[indexOfV1].y, vertexNormalList[indexOfV1].z );	
			glVertex3f( vertexList[indexOfV1].x, vertexList[indexOfV1].y, vertexList[indexOfV1].z );
		
			glTexCoord2f(vertexList[indexOfV2].x,vertexList[indexOfV2].y);
			glNormal3f( vertexNormalList[indexOfV2].x, vertexNormalList[indexOfV2].y, vertexNormalList[indexOfV2].z );
			glVertex3f( vertexList[indexOfV2].x, vertexList[indexOfV2].y, vertexList[indexOfV2].z );
		
			glTexCoord2f(vertexList[indexOfV3].x,vertexList[indexOfV3].y);
			glNormal3f( vertexNormalList[indexOfV3].x, vertexNormalList[indexOfV3].y, vertexNormalList[indexOfV3].z );
			glVertex3f( vertexList[indexOfV3].x, vertexList[indexOfV3].y, vertexList[indexOfV3].z );
		
		}
	glEnd();
	
	//glFlush();
	glutSwapBuffers();	// swap front/back framebuffer to avoid flickering 

	curClock=clock();
	float elapsed=(curClock-startClock)/(float)CLOCKS_PER_SEC;
	if(elapsed>1.0f){
		prevF=curF;
		startClock=curClock;
	}
}
 
void reshape (int w, int h)
{
	// reset viewport ( drawing screen ) size
	glViewport(0, 0, w, h);
	float fAspect = ((float)w)/h; 
	// reset OpenGL projection matrix
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(70.f,fAspect,0.001f,30.f); 
}



void keyboard(unsigned char key, int x, int y)
{
	if( key == 'p'){
		dipMode = 1-dipMode;
	}
	if( key == 'n'){
		normal = 1-normal;
	}
}


void timer(int v)
{
	fRotateAngle += 1.f; // change rotation angles
	glutPostRedisplay(); // trigger display function by sending redraw into message queue
	glutTimerFunc(1000/nFPS,timer,v); // restart timer again
}

int main(int argc, char* argv[])
{
	glutInit(&argc, (char**)argv);
	// set up for double-buffering & RGB color buffer & depth test
	glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH ); 
	glutInitWindowSize (500, 500); 
	glutInitWindowPosition (100, 100);
	glutCreateWindow ((const char*)"Teapot");

	glewInit(); //for texture mapping
	init(); // setting up user data & OpenGL environment
	
	// set up the call-back functions 
	glutDisplayFunc(display);  // called when drawing 
	glutReshapeFunc(reshape);  // called when change window size
	glutKeyboardFunc(keyboard); // called when received keyboard interaction
	glutTimerFunc(100,timer,nFPS); // a periodic timer. Usually used for updating animation
	atexit(program_end); //to free all malloc'd memory.
	
	startClock=clock();

	glutMainLoop(); // start the main message-callback loop

	return 0;
}
