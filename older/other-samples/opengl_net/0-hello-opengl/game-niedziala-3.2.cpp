#define GLEW_STATIC
// #include <GL/glew.h>
#include <GL/glu.h>
#include <GL/gl.h>
#include <SDL.h>
#include <SDL_opengl.h>


#include <vector>
#include <map>
#include <functional>
#include <chrono>
#include <iostream>
#include <thread>

//#include <veclib/veclib.hpp>

using std::vector;
using std::map;
using std::function;
using std::this_thread::sleep_for;
using std::chrono::milliseconds;
using std::chrono::microseconds;
using std::chrono::steady_clock;

// API OpenGL, które nie jest dostępne natychmiast po załączeniu plików nagłówkowych GL
// Można zamiast rozwiazania pokazanego poniżej, zastosować bibliotekę glew

typedef GLuint ( APIENTRY * GL_CreateProgram_Func )( void );
GL_CreateProgram_Func glCreateProgram = 0;
typedef GLuint ( APIENTRY * GL_CreateShader_Func )( GLenum shaderType );
GL_CreateShader_Func glCreateShader = 0;
typedef void ( APIENTRY * GL_ShaderSource_Func )( GLuint shader,GLsizei count,const GLchar **string,const GLint *length );
GL_ShaderSource_Func glShaderSource = 0;
typedef void ( APIENTRY * GL_CompileShader_Func )( GLuint shader );
GL_CompileShader_Func glCompileShader = 0;
typedef void ( APIENTRY * GL_GetShaderiv_Func )( GLuint shader,  GLenum pname,  GLint *params );
GL_GetShaderiv_Func glGetShaderiv = 0;



typedef void  ( APIENTRY * GL_AttachShader_Func     )( GLuint program, GLuint shader );
typedef void  ( APIENTRY * GL_LinkProgram_Func      )( GLuint program );
typedef void  ( APIENTRY * GL_GetProgramiv_Func     )( GLuint program, GLenum pname,GLint *params );
typedef GLint ( APIENTRY * GL_GetAttribLocation_Func )( GLuint program, const GLchar *name );
typedef void  ( APIENTRY * GL_GenBuffers_Func       )( GLsizei n,  GLuint * buffers );
typedef void  ( APIENTRY * GL_BindBuffer_Func       )( GLenum target, GLuint buffer );
typedef void  ( APIENTRY * GL_BufferData_Func       )( GLenum target, GLsizeiptr size, const GLvoid * data, GLenum usage );
typedef void  ( APIENTRY * GL_NamedBufferData_Func  )( GLuint buffer, GLsizei size, const void *data, GLenum usage );

GL_AttachShader_Func      glAttachShader      = 0;
GL_LinkProgram_Func       glLinkProgram       = 0;
GL_GetProgramiv_Func      glGetProgramiv      = 0;
GL_GetAttribLocation_Func glGetAttribLocation = 0;
GL_GenBuffers_Func        glGenBuffers        = 0;
GL_BindBuffer_Func        glBindBuffer        = 0;
GL_BufferData_Func        glBufferData        = 0;
GL_NamedBufferData_Func   glNamedBufferData   = 0;


void loadOpenGLlibrary() {
	glCreateProgram=( GL_CreateProgram_Func ) SDL_GL_GetProcAddress( "glCreateProgram" );
	glCreateShader=( GL_CreateShader_Func ) SDL_GL_GetProcAddress( "glCreateShader" );
	glShaderSource=( GL_ShaderSource_Func ) SDL_GL_GetProcAddress( "glShaderSource" );
	glCompileShader=( GL_CompileShader_Func ) SDL_GL_GetProcAddress( "glCompileShader" );
	glGetShaderiv=( GL_GetShaderiv_Func ) SDL_GL_GetProcAddress( "glGetShaderiv" );


	glAttachShader      =( GL_AttachShader_Func     ) SDL_GL_GetProcAddress( "glAttachShader"     );
	glLinkProgram       =( GL_LinkProgram_Func      ) SDL_GL_GetProcAddress( "glLinkProgram"      );
	glGetProgramiv      =( GL_GetProgramiv_Func     ) SDL_GL_GetProcAddress( "glGetProgramiv"     );
	glGetAttribLocation =( GL_GetAttribLocation_Func ) SDL_GL_GetProcAddress( "glGetAttribLocation" );
	glGenBuffers        =( GL_GenBuffers_Func       ) SDL_GL_GetProcAddress( "glGenBuffers"       );
	glBindBuffer        =( GL_BindBuffer_Func       ) SDL_GL_GetProcAddress( "glBindBuffer"       );
	glBufferData        =( GL_BufferData_Func       ) SDL_GL_GetProcAddress( "glBufferData"       );
	glNamedBufferData   =( GL_NamedBufferData_Func  ) SDL_GL_GetProcAddress( "glNamedBufferData"  );
}


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


	//Graphics program
	GLuint gProgramID = 0;
	GLint gVertexPos2DLocation = -1;
	GLuint gVBO = 0; // vertex buffer object
	GLuint gIBO = 0; // index buffer object

	vector < double > camera;

} SimulationState;

SimulationState simulationState;

int doGraphics() {
	// ustawienie kamery


	// wyswietlenie sceny
	glClear( GL_COLOR_BUFFER_BIT );
	//Render quad
	if( gRenderQuad ) {
		//Bind program
		glUseProgram( gProgramID );

		//Enable vertex position
		glEnableVertexAttribArray( gVertexPos2DLocation );

		//Set vertex data
		glBindBuffer( GL_ARRAY_BUFFER, gVBO );
		glVertexAttribPointer( gVertexPos2DLocation, 2, GL_FLOAT, GL_FALSE, 2 * sizeof( GLfloat ), NULL );

		//Set index data and render
		glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, gIBO );
		glDrawElements( GL_TRIANGLE_FAN, 4, GL_UNSIGNED_INT, NULL );

		//Disable vertex position
		glDisableVertexAttribArray( gVertexPos2DLocation );

		//Unbind program
		glUseProgram( NULL );
	}


	// rysujemy

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

//	// sterowanie
//	const Uint8 *state = SDL_GetKeyboardState(NULL);
//	// zerujemy aktuatory
//	ddp[0] = 0;
//	ddp[1] = 0;
//	// sprawdzamy klawisze
//	if (state[SDL_SCANCODE_LEFT]) ddp[0] = -10;
//	if (state[SDL_SCANCODE_RIGHT]) ddp[0] = 10;
//	if (state[SDL_SCANCODE_UP]) ddp[1] = 10;
//	if (state[SDL_SCANCODE_DOWN]) ddp[1] = -10;
}
int doPhysics( double dt ) {

}


int loadLevel() {
	SDL_Init( SDL_INIT_EVERYTHING );
	SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 3 );
	SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 2 );
	SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );
	SDL_GL_SetSwapInterval( 1 ); //

	simulationState.window = SDL_CreateWindow( "Prosta obsluga OpenGL", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 480, SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL );
	simulationState.glContext = SDL_GL_CreateContext( simulationState.window );
	simulationState.quit = false;
	simulationState.camera = {0,0,0};

	// zaladowanie funkcji dla OpenGL
	loadOpenGLlibrary();

	simulationState.gProgramID = glCreateProgram();
	GLuint vertexShader = glCreateShader( GL_VERTEX_SHADER );
	//Get vertex source
	const GLchar* vertexShaderSource[] = {
		"#version 140\nin vec2 LVertexPos2D; void main() { gl_Position = vec4( LVertexPos2D.x, LVertexPos2D.y, 0, 1 ); }"
	};
	//Set vertex source
	glShaderSource( vertexShader, 1, vertexShaderSource, NULL );
	//Compile vertex source
	glCompileShader( vertexShader );
	//Check vertex shader for errors
	GLint vShaderCompiled = GL_FALSE;
	glGetShaderiv( vertexShader, GL_COMPILE_STATUS, &vShaderCompiled );
	if( vShaderCompiled != GL_TRUE ) {
		printf( "Unable to compile vertex shader %d!\n", vertexShader );
		//printShaderLog( vertexShader );
		return -1; // BLAD!
	} else {
		//Attach vertex shader to program
		glAttachShader( simulationState.gProgramID, vertexShader );


		//Create fragment shader
		GLuint fragmentShader = glCreateShader( GL_FRAGMENT_SHADER );

		//Get fragment source
		const GLchar* fragmentShaderSource[] = {
			"#version 140\nout vec4 LFragment; void main() { LFragment = vec4( 1.0, 1.0, 1.0, 1.0 ); }"
		};

		//Set fragment source
		glShaderSource( fragmentShader, 1, fragmentShaderSource, NULL );

		//Compile fragment source
		glCompileShader( fragmentShader );

		//Check fragment shader for errors
		GLint fShaderCompiled = GL_FALSE;
		glGetShaderiv( fragmentShader, GL_COMPILE_STATUS, &fShaderCompiled );
		if( fShaderCompiled != GL_TRUE ) {
			printf( "Unable to compile fragment shader %d!\n", fragmentShader );
			//printShaderLog( fragmentShader );
			return -2; // BLAD!
		} else {
			//Attach fragment shader to program
			glAttachShader( simulationState.gProgramID, fragmentShader );

			//Link program
			glLinkProgram( simulationState.gProgramID );

			//Check for errors
			GLint programSuccess = GL_TRUE;
			glGetProgramiv( simulationState.gProgramID, GL_LINK_STATUS, &programSuccess );
			if( programSuccess != GL_TRUE ) {
				printf( "Error linking program %d!\n", simulationState.gProgramID );
				//printProgramLog( simulationState.gProgramID );
				return -3; // BLAD!
			} else {
				//Get vertex attribute location
				simulationState.gVertexPos2DLocation = glGetAttribLocation( simulationState.gProgramID, "LVertexPos2D" );
				if( simulationState.gVertexPos2DLocation == -1 ) {
					printf( "LVertexPos2D is not a valid glsl program variable!\n" );
					return -4; // BLAD!

				} else {
					//Initialize clear color
					glClearColor( 0.f, 0.f, 0.f, 1.f );

					//VBO data
					GLfloat vertexData[] = {
						-0.5f, -0.5f,
						0.5f, -0.5f,
						0.5f,  0.5f,
						-0.5f,  0.5f
					};

					//IBO data
					GLuint indexData[] = { 0, 1, 2, 3 };

					//Create VBO
					glGenBuffers( 1, &simulationState.gVBO );
					glBindBuffer( GL_ARRAY_BUFFER, simulationState.gVBO );
					glBufferData( GL_ARRAY_BUFFER, 2 * 4 * sizeof( GLfloat ), vertexData, GL_STATIC_DRAW );

					//Create IBO
					glGenBuffers( 1, &simulationState.gIBO );
					glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, simulationState.gIBO );
					glBufferData( GL_ELEMENT_ARRAY_BUFFER, 4 * sizeof( GLuint ), indexData, GL_STATIC_DRAW );
				}
			}
		}
	}
}

int main( int argc, char* args[] ) {
	double fps=60.0; // 60FPS i tickrate
	double dt=1.0/fps; // 60FPS


	loadLevel();

	auto prevTime = std::chrono::steady_clock::now();
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
		auto sleepTime = microseconds( ( int )( 1000000.0*( ( ( double )frame )/fps - ( ( ( std::chrono::duration<double> )( steady_clock::now() - t0 ) ).count() ) ) ) );
		if ( sleepTime.count() < 0 ) { // oj - nie zdazylismy wyswietlic klatki w zadanym czasie!!
			dropFrame = true;
		} else {
			dropFrame = false;
			sleep_for ( sleepTime );
		}
		auto now = steady_clock::now();
		cdt = now - prevTime; // przyrost czasu w sekundach
		prevTime = now;
		if ( dropFrame )
			std::cout << "FPS-dropped: " << frame << std::endl;
		/*		else
					std::cout << "FPS: " << (1.0/cdt.count()) << std::endl; */

		doUserInteractions( dt );

		doPhysics( dt );

		if ( frame > 100000000 ) { // zabezpieczenie przed przekręceniem licznika
			prevTime = std::chrono::steady_clock::now();
			t0 = prevTime;
			frame = 0;
		}

	}

	SDL_DestroyWindow( simulationState.window );
	SDL_Quit();
	return 0;
}

