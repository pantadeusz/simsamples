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
#include <string>
#include <map>
#include <functional>
#include <chrono>
#include <iostream>
#include <thread>

using namespace std;
using std::this_thread::sleep_for;
using std::chrono::milliseconds;
using std::chrono::microseconds;
using std::chrono::system_clock;

/**
 * Uwaga:
 * Program jest przykładem, uproszczenia zastosowane, których należy unikać w wielkich projektach to, między innymi, korzystanie ze zmiennych globalnych.
 * */

typedef class EngineState {
public:
	bool quit;
	SDL_Window* window;
	SDL_GLContext glContext;
	SDL_Event sdlEvent;
	vector < double > cameraPosition;
	vector < double > cameraRotation;
	vector < GLuint > textures;
} EngineState;

EngineState engineState;

void drawCube() {
	GLfloat vbo[] = {-0.5,-0.5,-0.5,  0.5,-0.5,-0.5,  0.5, 0.5,-0.5, -0.5, 0.5,-0.5,
	                 -0.5,-0.5, 0.5,  0.5,-0.5, 0.5,  0.5, 0.5, 0.5, -0.5, 0.5, 0.5,
	                 -0.5,-0.5,-0.5,  0.5,-0.5,-0.5,  0.5,-0.5, 0.5, -0.5,-0.5, 0.5,
	                 -0.5, 0.5,-0.5,	 0.5, 0.5,-0.5,	 0.5, 0.5, 0.5,	-0.5, 0.5, 0.5,
	                 -0.5,-0.5,-0.5,	-0.5, 0.5,-0.5, -0.5, 0.5, 0.5,	-0.5,-0.5, 0.5,
	                 0.5,-0.5,-0.5,	 0.5, 0.5,-0.5,	 0.5, 0.5, 0.5,	 0.5,-0.5, 0.5
	                }; // wierzcholki
	GLfloat tcbo[] = {0,0,1,0,1,1,0,1,
	                  0,0,1,0,1,1,0,1,
	                  0,0,1,0,1,0,0,0,
	                  0,1,1,1,1,1,0,1,
	                  0,0,0,1,0,1,0,0,
	                  1,0,1,1,1,1,1,0
	                 }; // wspolrzedne tekstury
	GLfloat cbo[] = {0,0.0,0.0, 1.0,0.0,0.0, 1.0,1.0,0.0, 0.0,1.0,0.0,
	                 0,0.0,1.0, 1.0,0.0,1.0, 1.0,1.0,1.0, 0.0,1.0,1.0,
	                 0,0.0,0.0, 1.0,0.0,0.0, 1.0,0.0,1.0, 0.0,0.0,1.0,
	                 0.0,1.0,0.0,	1.0,1.0,0.0,	1.0,1.0,1.0,	0.0,1.0,1.0,
	                 0.0,0.0,0.0,	0.0,1.0,0.0, 0.0,1.0,1.0,	0.0,0.0,1.0,
	                 1.0,0.0,0.0,	1.0,1.0,0.0,	1.0,1.0,1.0,	1.0,0.0,1.0
	                }; // kolorki
	for ( int i = 0; i < 6; i++ ) {
		glBegin( GL_POLYGON );
		for ( int j = 0; j < 4; j++ ) {
			glTexCoord2f( tcbo[i*4*2+j*2], tcbo[i*4*2+j*2+1] );
			glColor3f( cbo[i*4*3+j*3], cbo[i*4*3+j*3+1], cbo[i*4*3+j*3+2] );
			glVertex3f( vbo[i*4*3+j*3], vbo[i*4*3+j*3+1], vbo[i*4*3+j*3+2] );
		}
		glEnd();
	}
}


/**
 * Funkcja laduje liste tekstur z plikow bmp.
 * nie obsluguje przezroczystosci
 * zwraca zainicjowana liste "nazw" tekstur w OpenGL
 * */
vector < GLuint > loadTextures( const vector < string > fnames ) {
	vector < GLuint > textures( fnames.size() );
	glGenTextures( fnames.size(), textures.data() );
	for ( int i = 0; i < fnames.size(); i++ ) {
		cout << "loadiong " << fnames[i].c_str() << "\n";
		SDL_Surface* surf = SDL_LoadBMP( fnames[i].c_str() );
		if ( surf==NULL ) {
			cerr << SDL_GetError() << "\n";
			return {};
		}
		GLenum data_fmt;
		Uint8 test = SDL_MapRGB( surf->format, 0xAA,0xBB,0xCC )&0xFF;
		if      ( test==0xAA ) data_fmt=         GL_RGB;
		else if ( test==0xCC ) data_fmt=0x80E0; //GL_BGR;
		else {
			cerr << "Nie wiem jakiego formatu jest to obraz - BGR czy RGB\n";
			return {};
		}
		cout << "fmt: " << data_fmt << "\n";
		glBindTexture( GL_TEXTURE_2D,textures[i] );
		glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, surf->w,surf->h, 0, data_fmt,GL_UNSIGNED_BYTE,surf->pixels );
		glTexParameteri( GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR );
		glTexParameteri( GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR );
		SDL_FreeSurface( surf );
	}
	return textures;
}

void deleteTextures( const vector < GLuint > &textures ) {
	glDeleteTextures( textures.size(),textures.data() );
}

int doGraphics() {
	// kamera
	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity( );

	glTranslatef( engineState.cameraPosition[0], engineState.cameraPosition[1], engineState.cameraPosition[2] );
	glRotatef( engineState.cameraRotation[0], 1.0, 0.0, 0.0 );
	glRotatef( engineState.cameraRotation[1], 0.0, 1.0, 0.0 );
	glRotatef( engineState.cameraRotation[2], 0.0, 0.0, 1.0 );

	// wyswietlenie sceny
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	glEnable( GL_TEXTURE_2D );
	glBindTexture( GL_TEXTURE_2D,engineState.textures[0] );
	drawCube();
	glFlush();

	SDL_GL_SwapWindow( engineState.window );
}
int doUserInteractions( double dt ) {
	// zdarzenia
	SDL_Event e;
	while( SDL_PollEvent( &e ) != 0 ) {
		if( e.type == SDL_QUIT ) {
			engineState.quit = true;
		}
	}

	const Uint8 *state = SDL_GetKeyboardState( NULL );
	vector < double > ddp = {0,0,0};
	if ( state[SDL_SCANCODE_LEFT] ) ddp[0] = -10;
	if ( state[SDL_SCANCODE_RIGHT] ) ddp[0] = 10;
	if ( state[SDL_SCANCODE_UP] ) ddp[1] = 10;
	if ( state[SDL_SCANCODE_DOWN] ) ddp[1] = -10;

	if ( state[SDL_SCANCODE_LSHIFT] ) {
		engineState.cameraRotation[0] += 4.0*ddp[0]*dt;
		engineState.cameraRotation[1] += 4.0*ddp[1]*dt;
	} else {
		engineState.cameraPosition[0] += ddp[0]*dt;
		engineState.cameraPosition[1] += ddp[1]*dt;
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

	engineState.window = SDL_CreateWindow( "SGD - Przyklad OpenGL", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL );
	engineState.glContext = SDL_GL_CreateContext( engineState.window );
	engineState.quit = false;
	engineState.cameraPosition = {0,0,-10};
	engineState.cameraRotation = {0,0,0};

	engineState.textures = loadTextures( {string( "data/piesel.bmp" )} );

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
	while ( !engineState.quit ) {
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
	deleteTextures( engineState.textures );
	SDL_DestroyWindow( engineState.window );
	SDL_Quit();
	return 0;
}

