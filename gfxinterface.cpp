#include <iostream>
#include "gfxmath.h"
#include "PSOStructures.h"
#include <sstream>
#include <vector>
#include <cmath>
#include <GL/gl.h>
#include <GL/glu.h>
#include "SDL.h"

#define SCREEN_WIDTH 1024
#define SCREEN_HEIGHT 768
#define SCREEN_BPP 16
#define PARTICLES 10000
#define TRUE 1
#define FALSE 0

using namespace std;
//Global variables
vector<Particle> pswarm = vector<Particle>(PARTICLES);
ParticleBest gbest;
bool draw = true;
bool showBest = true;
bool showPosition = true;
float xrot = 0.0f;
float yrot = 0.0f;
float zrot = 0.0f;
float dist = 5.0f;
int selectObjectUID = -1;
SDL_Surface *surface;
void updateSwarm(vector<Particle> &swarm,ParticleBest &gbest);

void Quit(int returnCode)
{
	SDL_Quit();

	exit(returnCode);
}

void ReSizeGLScene(int width,int height)
{
	if (height == 0)
		height = 1;
	GLfloat ratio = static_cast<GLfloat>(width) / static_cast<GLfloat>(height);
	glViewport(0,0, width,height);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	gluPerspective(45.0f,ratio,0.1f,100.0f);
	
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

}

void keyPressed(SDL_keysym *keysym)
{
	switch(keysym->sym)
	{
		case SDLK_ESCAPE:
			Quit(0);
			break;
		case SDLK_F1:
			draw = !draw;	
			selectObjectUID = -1;
			break;
		case SDLK_F2:
			showBest = !showBest;
			break;
		case SDLK_F3:
			showPosition = !showPosition;
			break;
		default:
			break;
	}
	return;
}

void initRandomParticle(vector<Particle>& swarm,const unsigned int& population)
{
	boost::mt19937 rng;
	boost::uniform_real<double> u(-5.14,5.14);
	boost::variate_generator<boost::mt19937&, boost::uniform_real<double> > gen(rng, u);
	for(unsigned int i =0; i < population;i++)
	{
		double x = gen();
		double y = gen();
		double z = gen();
		swarm[i] = Particle(Vector3d(x,y,z),i);
	}
}

void evaluateSwarm(vector<Particle>& swarm,ParticleBest& cgbest)
{
	for(unsigned int i = 0;i < swarm.size();i++)
	{
		if(swarm[i].getBestFitness() < cgbest.fitness)
		{
			cgbest = swarm[i].getParticleBest();
		}
	}
	cout <<"Current best fitness is : " << cgbest.fitness << endl;
}

void updateSwarm(vector<Particle> &swarm,ParticleBest &gbest)
{	
	double c1 = 0.41;
	double c2 = 0.52;
	boost::mt19937 rng;
	boost::uniform_real<double> u(0.00,1.00);
	boost::variate_generator<boost::mt19937&, boost::uniform_real<double> > gen(rng, u);	
	for(unsigned int i =0; i < swarm.size();i++)
	{
		swarm[i].velocity = swarm[i].velocity + c1 * gen() * (swarm[i].getBestPosition() - swarm[i].getPosition()) + c2 * (gbest.pos - swarm[i].getPosition());
		swarm[i].move();
	}
	evaluateSwarm(swarm,gbest);
}

void setupPSO(vector<Particle> &swarm,const int& population,ParticleBest &gbest)
{
	initRandomParticle(swarm,population);
	gbest = swarm[0].getParticleBest();
	evaluateSwarm(swarm,gbest);
}

int InitGL()
{
	glClearColor(0.0f,0.0f,0.0f,0.0f);

	glClearDepth(1.0f);

	glDepthFunc(GL_LESS);
	glEnable(GL_DEPTH_TEST);

	glShadeModel(GL_SMOOTH);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	gluPerspective(45.0f,(GLfloat)SCREEN_WIDTH/(GLfloat)SCREEN_HEIGHT,0.1f,100.0f);
	
	glMatrixMode(GL_MODELVIEW);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

	return TRUE;
}

void DrawGLScene(int hitUID)
{
	static GLint Frames = 0;
	GLint mode;
	GLfloat sizes[2];
	GLfloat step;
	GLfloat curSize;

	glGetIntegerv(GL_RENDER_MODE,&mode);
	glGetFloatv(GL_POINT_SIZE_RANGE,sizes);
	glGetFloatv(GL_POINT_SIZE_GRANULARITY,&step);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(0.0,0.0,dist,0.0,0.0,0.0,0.0,1.0,0.0);
	glRotatef(xrot,1.0,0.0,0.0);
	glRotatef(yrot,0.0,1.0,0.0);
	//Set point size
	//curSize = sizes[0]+5;
	//glPointSize(curSize);
	//Draw minimum point
	glBegin(GL_QUADS);
	glColor3f(0.5,0.5,0.5);
	GLfloat point = 0.015;
	glVertex3f(-point,point,-0);
	glVertex3f(point,point,-0);
	glVertex3f(point,-point,-0);
	glVertex3f(-point,-point,-0);
	glEnd();
	//Draw the particles
	point = 0.005;
	for (unsigned int i =0; i < pswarm.size();i++)
	{
		Vector3d pos;
		if (mode == GL_SELECT) glLoadName(pswarm[i].getUID());
		if (showBest  == true)
		{
			pos = pswarm[i].getBestPosition();
			glColor3f(1.0,1.0,1.0);
		glBegin(GL_QUADS);
			//front face
			glVertex3f(pos.x-point,pos.y+point,pos.z+point);
			glVertex3f(pos.x+point,pos.y+point,pos.z+point);
			glVertex3f(pos.x+point,pos.y-point,pos.z+point);
			glVertex3f(pos.x-point,pos.y-point,pos.z+point);
			//backface
			glVertex3f(pos.x-point,pos.y+point,pos.z-point);
			glVertex3f(pos.x+point,pos.y+point,pos.z-point);
			glVertex3f(pos.x+point,pos.y-point,pos.z-point);
			glVertex3f(pos.x-point,pos.y-point,pos.z-point);
			//right face
			glVertex3f(pos.x+point,pos.y+point,pos.z-point);
			glVertex3f(pos.x+point,pos.y+point,pos.z+point);
			glVertex3f(pos.x+point,pos.y-point,pos.z+point);
			glVertex3f(pos.x+point,pos.y-point,pos.z-point);
			//Left face	
			glVertex3f(pos.x-point,pos.y+point,pos.z-point);
			glVertex3f(pos.x-point,pos.y+point,pos.z+point);
			glVertex3f(pos.x-point,pos.y-point,pos.z+point);
			glVertex3f(pos.x-point,pos.y-point,pos.z-point);
			//Top face	
			glVertex3f(pos.x-point,pos.y+point,pos.z-point);
			glVertex3f(pos.x-point,pos.y+point,pos.z+point);
			glVertex3f(pos.x-point,pos.y+point,pos.z+point);
			glVertex3f(pos.x-point,pos.y+point,pos.z-point);
			//Bottom face	
			glVertex3f(pos.x-point,pos.y-point,pos.z-point);
			glVertex3f(pos.x-point,pos.y-point,pos.z+point);
			glVertex3f(pos.x-point,pos.y-point,pos.z+point);
			glVertex3f(pos.x-point,pos.y-point,pos.z-point);
		glEnd();
		}
		if (showPosition == true)
		{
			pos = pswarm[i].getPosition();
			glColor3f(1 - pos.x,1 - pos.y,1 - pos.z);
			glBegin(GL_QUADS);
			//front face
			glVertex3f(pos.x-point,pos.y+point,pos.z+point);
			glVertex3f(pos.x+point,pos.y+point,pos.z+point);
			glVertex3f(pos.x+point,pos.y-point,pos.z+point);
			glVertex3f(pos.x-point,pos.y-point,pos.z+point);
			//backface
			glVertex3f(pos.x-point,pos.y+point,pos.z-point);
			glVertex3f(pos.x+point,pos.y+point,pos.z-point);
			glVertex3f(pos.x+point,pos.y-point,pos.z-point);
			glVertex3f(pos.x-point,pos.y-point,pos.z-point);
			//right face
			glVertex3f(pos.x+point,pos.y+point,pos.z-point);
			glVertex3f(pos.x+point,pos.y+point,pos.z+point);
			glVertex3f(pos.x+point,pos.y-point,pos.z+point);
			glVertex3f(pos.x+point,pos.y-point,pos.z-point);
			//Left face	
			glVertex3f(pos.x-point,pos.y+point,pos.z-point);
			glVertex3f(pos.x-point,pos.y+point,pos.z+point);
			glVertex3f(pos.x-point,pos.y-point,pos.z+point);
			glVertex3f(pos.x-point,pos.y-point,pos.z-point);
			//Top face	
			glVertex3f(pos.x-point,pos.y+point,pos.z-point);
			glVertex3f(pos.x-point,pos.y+point,pos.z+point);
			glVertex3f(pos.x-point,pos.y+point,pos.z+point);
			glVertex3f(pos.x-point,pos.y+point,pos.z-point);
			//Bottom face	
			glVertex3f(pos.x-point,pos.y-point,pos.z-point);
			glVertex3f(pos.x-point,pos.y-point,pos.z+point);
			glVertex3f(pos.x-point,pos.y-point,pos.z+point);
			glVertex3f(pos.x-point,pos.y-point,pos.z-point);
			glEnd();
		}
		glEnd();
		if (mode == GL_SELECT) glPopName();
	}
	if (mode == GL_RENDER) SDL_GL_SwapBuffers();

}

int doSelect(const double &x,const double &y)
{
	GLuint buff[64] = {0};
	GLint hits,view[4];
	
	glSelectBuffer(64,buff);

	glRenderMode(GL_SELECT);

	glInitNames();
	glPushName(0);

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
		glLoadIdentity();

		glGetIntegerv(GL_VIEWPORT,view);
		gluPickMatrix(x,y,50.0,50.0,view);//gl (0,0) bottom left, window (0,0) top left, that is why view[3] -y
		gluPerspective(45.0f,(GLfloat)SCREEN_WIDTH /(GLfloat) SCREEN_HEIGHT,0.1f,100.0f);

		DrawGLScene(-1);//-1 indicates ignore special UID rendering
		glMatrixMode(GL_PROJECTION);
	glPopMatrix();

    hits = glRenderMode(GL_RENDER);	
	//get nearest hit
	GLuint *hitptr = buff;
	float minz = hitptr[0 * 4 + 1];	
	int ruid = hitptr[0 * 4];
	cout << "HITS:" << hits<< endl;
	for(int i = 0; i < hits;i++)
	{
		cout << hitptr[0 * 4] << endl;
		if (hitptr[i * 4 + 1] < minz)
		{
			minz = hitptr[i * 4 + 1];
			ruid = hitptr[i * 4];
		}
	}
	return ruid;
}

int main(int argc, char** argv)
{
	setupPSO(pswarm,PARTICLES,gbest);
	int videoFlags;
	float tick = 0.25;
	bool done=false;
	bool calcRotation = false;
	SDL_Event event;
	const SDL_VideoInfo *videoInfo;

	if(SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		cout << "Video initialization failed: " << SDL_GetError();
		Quit(1);
	}
	videoInfo = SDL_GetVideoInfo();

	if(!videoInfo)
	{
		cout << "Video query failed:" << SDL_GetError();
		Quit(1);
	}
	// Flags to pass to SDL_SetVideoMode
	videoFlags = SDL_OPENGL;
	videoFlags |= SDL_GL_DOUBLEBUFFER;
	videoFlags |= SDL_HWPALETTE;
	videoFlags |= SDL_RESIZABLE;

	//Check to see if the surfaces can be stored in memory
	if(videoInfo->hw_available)
	{
		videoFlags |= SDL_HWSURFACE;
	}
	else
	{
		videoFlags |= SDL_SWSURFACE;
	}

	if(videoInfo->blit_hw)
	{
		videoFlags |= SDL_HWACCEL;
	}
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER,1);

	surface = SDL_SetVideoMode(SCREEN_WIDTH,SCREEN_HEIGHT,SCREEN_BPP,videoFlags);

	if(InitGL() != TRUE)
	{
		cout << "Error initializing OpenGL" << endl;
		Quit(1);
	}
	
	while(!done)
	{
		while(SDL_PollEvent(&event))
		{
			switch(event.type)
			{
				case SDL_ACTIVEEVENT:
					//ACTIVEEVENT = Lose and gain focus
					break;
				case SDL_KEYDOWN:
					keyPressed(&event.key.keysym);
					break;
				case SDL_MOUSEBUTTONDOWN:
					if(event.button.button == SDL_BUTTON_RIGHT)
						selectObjectUID = doSelect(event.button.x,event.button.y);
					if(event.button.button == SDL_BUTTON_LEFT)
						calcRotation=true;
					if(event.button.button == SDL_BUTTON_WHEELUP)
						dist -= tick;
					if(event.button.button == SDL_BUTTON_WHEELDOWN)
						dist += tick;
					break;
				case SDL_MOUSEBUTTONUP:
					if (event.button.button == SDL_BUTTON_LEFT)
						calcRotation=false;
					break;
				case SDL_MOUSEMOTION:
					if (calcRotation==true)
					{
						xrot += event.motion.yrel;
						yrot += event.motion.xrel;
					}
					break;
				case SDL_QUIT:
					done = true;
					break;
				default:
					break;
			}
		}
		DrawGLScene(selectObjectUID);
		if (draw == false)
		{
			updateSwarm(pswarm,gbest);	
		}
	}
	return(0);
}
