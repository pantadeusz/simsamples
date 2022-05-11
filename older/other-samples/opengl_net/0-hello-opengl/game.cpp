/**
 * Przykład pokazujący prosty programik korzystający z OpenGL do wyświetlania kostki.
 *
 * Omówione na wykładzie.
 *
 * */

#define GL_GLEXT_PROTOTYPES

//#include <GL/glu.h>
#include <GL/gl.h>
#include <SDL.h>
#include <vector>
#include <map>
#include <functional>
#include <chrono>
#include <iostream>
#include <thread>

using std::vector;
using std::map;
using std::function;
using std::this_thread::sleep_for;
using std::chrono::milliseconds;
using std::chrono::microseconds;
using std::chrono::system_clock;

/**
 * Uwaga:
 * Program jest przykładem, uproszczenia zastosowane, których należy unikać w wielkich projektach to, między innymi, korzystanie ze zmiennych globalnych.
 * */

typedef class SimulationState {
public:
	bool quit;
	SDL_Window* window;
	SDL_GLContext glContext;
	SDL_Event sdlEvent;
	vector < double > cameraPosition;
	vector < double > cameraRotation;
} SimulationState;

SimulationState simulationState;

void drawCube() {
	double vbo[] = {-0.5,-0.5,-0.5,  0.5,-0.5,-0.5,  0.5, 0.5,-0.5, -0.5, 0.5,-0.5,
	                -0.5,-0.5, 0.5,  0.5,-0.5, 0.5,  0.5, 0.5, 0.5, -0.5, 0.5, 0.5,
	                -0.5,-0.5,-0.5,  0.5,-0.5,-0.5,  0.5,-0.5, 0.5, -0.5,-0.5, 0.5,
	                -0.5, 0.5,-0.5,	 0.5, 0.5,-0.5,	 0.5, 0.5, 0.5,	-0.5, 0.5, 0.5,
	                -0.5,-0.5,-0.5,	-0.5, 0.5,-0.5, -0.5, 0.5, 0.5,	-0.5,-0.5, 0.5,
	                0.5,-0.5,-0.5,	 0.5, 0.5,-0.5,	 0.5, 0.5, 0.5,	 0.5,-0.5, 0.5
	               }; // wierzcholki
	double cbo[] = {0.5+-0.5,0.5+-0.5,0.5+-0.5, 0.5+ 0.5,0.5+-0.5,0.5+-0.5, 0.5+ 0.5,0.5+ 0.5,0.5+-0.5, 0.5+-0.5,0.5+ 0.5,0.5+-0.5,
	                0.5+-0.5,0.5+-0.5,0.5+ 0.5, 0.5+ 0.5,0.5+-0.5,0.5+ 0.5, 0.5+ 0.5,0.5+ 0.5,0.5+ 0.5, 0.5+-0.5,0.5+ 0.5,0.5+ 0.5,
	                0.5+-0.5,0.5+-0.5,0.5+-0.5, 0.5+ 0.5,0.5+-0.5,0.5+-0.5, 0.5+ 0.5,0.5+-0.5,0.5+ 0.5, 0.5+-0.5,0.5+-0.5,0.5+ 0.5,
	                0.5+-0.5,0.5+ 0.5,0.5+-0.5,	0.5+ 0.5,0.5+ 0.5,0.5+-0.5,	0.5+ 0.5,0.5+ 0.5,0.5+ 0.5,	0.5+-0.5,0.5+ 0.5,0.5+ 0.5,
	                0.5+-0.5,0.5+-0.5,0.5+-0.5,	0.5+-0.5,0.5+ 0.5,0.5+-0.5, 0.5+-0.5,0.5+ 0.5,0.5+ 0.5,	0.5+-0.5,0.5+-0.5,0.5+ 0.5,
	                0.5+ 0.5,0.5+-0.5,0.5+-0.5,	0.5+ 0.5,0.5+ 0.5,0.5+-0.5,	0.5+ 0.5,0.5+ 0.5,0.5+ 0.5,	0.5+ 0.5,0.5+-0.5,0.5+ 0.5
	               }; // kolorki
	for ( int i = 0; i < 6; i++ ) {
		glBegin( GL_POLYGON );
		for ( int j = 0; j < 4; j++ ) {
			glColor3f( cbo[i*4*3+j*3], cbo[i*4*3+j*3+1], cbo[i*4*3+j*3+2] );
			glVertex3f( vbo[i*4*3+j*3], vbo[i*4*3+j*3+1], vbo[i*4*3+j*3+2] );
		}
		glEnd();
	}
}

int doGraphics() {
	// kamera
	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity( );

	glTranslatef( simulationState.cameraPosition[0], simulationState.cameraPosition[1], simulationState.cameraPosition[2] );
	glRotatef( simulationState.cameraRotation[0], 1.0, 0.0, 0.0 );
	glRotatef( simulationState.cameraRotation[1], 0.0, 1.0, 0.0 );
	glRotatef( simulationState.cameraRotation[2], 0.0, 0.0, 1.0 );

	// wyswietlenie sceny
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	drawCube();
	glFlush();

	SDL_GL_SwapWindow( simulationState.window );
}
int doUserInteractions( double dt ) {
	// zdarzenia
	SDL_Event e;
	while( SDL_PollEvent( &e ) != 0 ) {
		if( e.type == SDL_QUIT ) {
			simulationState.quit = true;
		}
	}

	const Uint8 *state = SDL_GetKeyboardState( NULL );
	vector < double > ddp = {0,0,0};
	if ( state[SDL_SCANCODE_LEFT] ) ddp[0] = -10;
	if ( state[SDL_SCANCODE_RIGHT] ) ddp[0] = 10;
	if ( state[SDL_SCANCODE_UP] ) ddp[1] = 10;
	if ( state[SDL_SCANCODE_DOWN] ) ddp[1] = -10;

	if ( state[SDL_SCANCODE_LSHIFT] ) {
		simulationState.cameraRotation[0] += 4.0*ddp[0]*dt;
		simulationState.cameraRotation[1] += 4.0*ddp[1]*dt;
	} else {
		simulationState.cameraPosition[0] += ddp[0]*dt;
		simulationState.cameraPosition[1] += ddp[1]*dt;
	}
}

int doPhysics( double dt ) {

}

void perspectiveGL( GLdouble fovY, GLdouble aspect, GLdouble zNear, GLdouble zFar ) {
	// za http://nehe.gamedev.net/article/replacement_for_gluperspective/21002/
	GLdouble fW, fH;
	fH = tan( fovY / 360 * 3.1415926535897932384626433832795 ) * zNear;
	fW = fH * aspect;
	glFrustum( -fW, fW, -fH, fH, zNear, zFar );
}


int loadLevel( int width = 640, int height = 480 ) {
	SDL_Init( SDL_INIT_EVERYTHING );

	simulationState.window = SDL_CreateWindow( "SGD - Przyklad OpenGL", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL );
	simulationState.glContext = SDL_GL_CreateContext( simulationState.window );
	simulationState.quit = false;
	simulationState.cameraPosition = {0,0,-10};
	simulationState.cameraRotation = {0,0,0};

	glShadeModel( GL_SMOOTH );
	glCullFace( GL_BACK );
	glFrontFace( GL_CCW );
	//glEnable( GL_CULL_FACE );
	glDisable( GL_CULL_FACE );
	glEnable( GL_DEPTH_TEST );
	glClearColor( 0, 0, 0, 0 );

	// widok
	glViewport( 0, 0, width, height );
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity( );
	perspectiveGL( 60.0, width/height, 1.0, 1024.0 );
}

int main( int argc, char* args[] ) {
	double fps=60.0; // 60FPS i tickrate
	double dt=1.0/fps; // 60FPS


	loadLevel();

	auto prevTime = std::chrono::system_clock::now();
	auto t0 = prevTime; // zero time
	std::chrono::duration<double> cdt; // frame time - calculated

	unsigned long int frame = 0;

	bool dropFrame = false;
	while ( !simulationState.quit ) {
		// stale framerate z mozliwoscia gubieniem klatek!
		if ( !dropFrame ) {
			doGraphics();
		}
		frame++;
		auto sleepTime = microseconds( ( int )( 1000000.0*( ( ( double )frame )/fps - ( ( ( std::chrono::duration<double> )( system_clock::now() - t0 ) ).count() ) ) ) );
		if ( sleepTime.count() < 0 ) { // oj - nie zdazylismy wyswietlic klatki w zadanym czasie!!
			dropFrame = true;
		} else {
			dropFrame = false;
			sleep_for ( sleepTime );
		}
		auto now = system_clock::now();
		cdt = now - prevTime; // przyrost czasu w sekundach
		prevTime = now;
		if ( dropFrame )
			std::cout << "FPS-dropped: " << frame << std::endl;
		/*		else
					std::cout << "FPS: " << (1.0/cdt.count()) << std::endl; */

		doUserInteractions( dt );

		doPhysics( dt );

		if ( frame > 100000000 ) { // zabezpieczenie przed przekręceniem licznika
			prevTime = std::chrono::system_clock::now();
			t0 = prevTime;
			frame = 0;
		}

	}

	SDL_DestroyWindow( simulationState.window );
	SDL_Quit();
	return 0;
}

