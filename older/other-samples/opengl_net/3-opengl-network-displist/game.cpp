/**
 * Przykład pokazujący prosty programik korzystający z OpenGL do wyświetlania kostki.
 *
 * Omówione na wykładzie.
 *
 * */
#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <SDL.h>
#include <vector>
#include <string>
#include <map>
#include <functional>
#include <chrono>
#include <iostream>
#include <thread>
#include <atomic>


#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <stdexcept>
#include <utility>
#include <sstream>


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
	vector < GLuint > vbos;  // video buffer object
	vector < vector < double >  > others;
} EngineState;

EngineState engineState;

void drawCube() {
	glBindBuffer( GL_ARRAY_BUFFER, engineState.vbos[2] );
	glNormalPointer( GL_FLOAT, 0, NULL );
	glEnableClientState( GL_NORMAL_ARRAY );

	glBindBuffer( GL_ARRAY_BUFFER, engineState.vbos[0] );
	glVertexPointer( 3, GL_FLOAT, 0, NULL );
	glEnableClientState( GL_VERTEX_ARRAY );

	glBindBuffer( GL_ARRAY_BUFFER, engineState.vbos[1] );
	glTexCoordPointer( 2, GL_FLOAT, 0, NULL );
	glEnableClientState( GL_TEXTURE_COORD_ARRAY );

	glDrawArrays( GL_QUADS, 0, 4*6 );


	GLubyte id[] = {0,1,2,3,
	                4,5,6,7,
	                0,1,5,4,
	                3,2,6,7,
	                0,3,7,4,
	                1,2,6,2
	               };
	glDrawElements( GL_QUADS,4*6,GL_UNSIGNED_BYTE,id );
}


/**
 * Funkcja laduje liste tekstur z plikow bmp.
 * nie obsluguje przezroczystosci
 * zwraca zainicjowana liste "nazw" tekstur w OpenGL
 * */
vector < GLuint > loadTextures( const vector < string > fnames ) {
	vector < GLuint > textures( fnames.size() );
	glGenTextures( fnames.size(),textures.data() );
	for ( int i = 0; i < fnames.size(); i++ ) {
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

	// wyswietlenie sceny
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	glEnable( GL_TEXTURE_2D );
	glEnable( GL_LIGHTING );
	glEnable( GL_LIGHT0 );
	glBindTexture( GL_TEXTURE_2D,engineState.textures[0] );


	glPushMatrix();
	glTranslatef( engineState.cameraPosition[0], engineState.cameraPosition[1], engineState.cameraPosition[2] );
	glRotatef( engineState.cameraRotation[0], 1.0, 0.0, 0.0 );
	glRotatef( engineState.cameraRotation[1], 0.0, 1.0, 0.0 );
	glRotatef( engineState.cameraRotation[2], 0.0, 0.0, 1.0 );
	drawCube();
	glPopMatrix();


	for ( int i = 0; i < engineState.others.size() ; i++ ) {
		glPushMatrix();
		glTranslatef( engineState.others[i][0], engineState.others[i][1], engineState.others[i][2] );
		drawCube();
		glPopMatrix();
	}

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

	GLfloat vbo[] = {-0.5,-0.5,-0.5,  0.5,-0.5,-0.5,  0.5, 0.5,-0.5, -0.5, 0.5,-0.5,
	                 -0.5,-0.5, 0.5,  0.5,-0.5, 0.5,  0.5, 0.5, 0.5, -0.5, 0.5, 0.5,
	                 -0.5,-0.5,-0.5,  0.5,-0.5,-0.5,  0.5,-0.5, 0.5, -0.5,-0.5, 0.5,
	                 -0.5, 0.5,-0.5,  0.5, 0.5,-0.5,  0.5, 0.5, 0.5, -0.5, 0.5, 0.5,
	                 -0.5,-0.5,-0.5, -0.5, 0.5,-0.5, -0.5, 0.5, 0.5, -0.5,-0.5, 0.5,
	                 0.5,-0.5,-0.5,  0.5, 0.5,-0.5,  0.5, 0.5, 0.5,  0.5,-0.5, 0.5
	                }; // wierzcholki
	GLfloat tcbo[] = {0,1, 1,1, 1,0, 0,0,
	                  0,1, 1,1, 1,0, 0,0,
	                  0,0, 1,0, 1,0, 0,0,
	                  0,1, 1,1, 1,1, 0,1,
	                  0,0, 0,1, 0,1, 0,0,
	                  1,0, 1,1, 1,1, 1,0
	                 }; // wspolrzedne tekstury
	GLfloat nbo[] = { 0.0, 0.0,-1.0,    0.0 , 0.0,-1.0,    0.0 , 0.0,-1.0,    0.0 , 0.0,-1.0,
	                  0.0, 0.0, 1.0, 0.0 , 0.0, 1.0, 0.0 , 0.0, 1.0, 0.0 , 0.0, 1.0,
	                  0.0,-1.0, 0.0, 0.0 ,-1.0, 0.0, 0.0 ,-1.0, 0.0, 0.0 ,-1.0, 0.0,
	                  0.0, 1.0, 0.0, 0.0 , 1.0, 0.0, 0.0 , 1.0, 0.0, 0.0 , 1.0, 0.0,
	                  -1.0, 0.0, 0.0, -1.0, 0.0, 0.0, -1.0, 0.0, 0.0, -1.0, 0.0, 0.0,
	                  1.0, 0.0, 0.0,  1.0, 0.0, 0.0,  1.0, 0.0, 0.0,  1.0, 0.0, 0.0
	                }; // normalne

	engineState.vbos.resize( 3 );
	glGenBuffers( 3, engineState.vbos.data() );
	glBindBuffer( GL_ARRAY_BUFFER, engineState.vbos[0] );
	glBufferData( GL_ARRAY_BUFFER, 3*4*6*sizeof( GLfloat ), vbo, GL_STATIC_DRAW );
	glBindBuffer( GL_ARRAY_BUFFER, engineState.vbos[1] );
	glBufferData( GL_ARRAY_BUFFER, 2*4*6*sizeof( GLfloat ), tcbo, GL_STATIC_DRAW );
	glBindBuffer( GL_ARRAY_BUFFER, engineState.vbos[2] );
	glBufferData( GL_ARRAY_BUFFER, 3*4*6*sizeof( GLfloat ), nbo, GL_STATIC_DRAW );
}

void unloadLevel() {
	glDeleteBuffers( 3,  engineState.vbos.data() );
	deleteTextures( engineState.textures );
}

///////////////////// KOD SIECIOWY /////////////////////////////

typedef class ClientConnection {
public:
	int s; // polaczone gniazdo
	struct sockaddr_in remoteAddr;
	socklen_t remoteAddrSize;

	vector < double > position;
} ClientConnection;

atomic_char serverThreadState( 0 );


void serverThread() {
	int port = 19191;
	vector < ClientConnection > connectionPool;

	int sockfd; // gniazdo nasluchujace
	int yes=1;
	struct sockaddr_in my_addr;
	if ( ( sockfd = socket( PF_INET, SOCK_STREAM, 0 ) ) == -1 ) {
		perror( "socket" );
		exit( 1 );
	}
	if ( setsockopt( sockfd,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof( int ) ) == -1 ) {
		perror( "setsockopt" );
		exit( 1 );
	}
	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons( port );
	my_addr.sin_addr.s_addr = INADDR_ANY;
	memset( &( my_addr.sin_zero ), '\0', 8 );

	if ( ::bind( sockfd, ( struct sockaddr * )&my_addr, sizeof( struct sockaddr ) ) == -1 ) {
		perror( "bind" );
		exit( 1 );
	}

	fcntl( sockfd, F_SETFD, O_NONBLOCK ); // od teraz nie blokujemy na akceptowaniu polaczenia
	if ( listen( sockfd, 32 ) == -1 ) {
		perror( "listen" );
		exit( 1 );
	}
	signal( SIGPIPE, SIG_IGN );
	cout << "Serwer na porcie: " << std::to_string( port ) << endl;

	double fps=60.0; // 60FPS i tickrate
	double dt=1.0/fps; // 60FPS

	auto prevTime = std::chrono::system_clock::now();
	auto t0 = prevTime; // czas startu
	std::chrono::duration<double> cdt; // obliczony czas ramki
	unsigned long long int frame = 0;
	bool dropFrame = false;
	serverThreadState = 1;
	while ( serverThreadState == 1 ) {

		if ( ( frame % 60 ) == 0 ) { // co sekunde sprawdzamy czy ktos sie podlancza
			// czekamy na nowych graczy
			fd_set set;
			struct timeval timeout;
			int rv;
			FD_ZERO( &set );
			FD_SET( sockfd, &set );
			timeout.tv_sec = 0;
			timeout.tv_usec = 0;
			rv = select( sockfd + 1, &set, NULL, NULL, &timeout );
			if( rv == -1 ) {
				perror( "select" );
				break;
			} else if( rv != 0 ) {
				ClientConnection newCc;
				newCc.remoteAddrSize = sizeof( struct sockaddr_in );
				cout << "czy Nowy piesel :D" << endl;
				if ( ( newCc.s = accept( sockfd, ( struct sockaddr * )&newCc.remoteAddr, &newCc.remoteAddrSize ) ) > 0 ) {
					// akceptacja polaczenia -- to jest lokalny gracz
					setsockopt( newCc.s, IPPROTO_TCP, TCP_NODELAY, ( char * ) &yes, sizeof( int ) );
					connectionPool.push_back( newCc );
					cout << "Nowy piesel :D" << endl;
				}
				prevTime = system_clock::now();
			}
		}

		frame++;

		// odbieramy potwierdzenia
		for ( int i = 0; i < connectionPool.size(); i++ ) {
			connectionPool[i].position.resize( 3 );
			auto ret = recv( connectionPool[i].s, connectionPool[i].position.data(), 3*sizeof( engineState.cameraPosition[0] ), MSG_NOSIGNAL );
			if ( ret < 3*sizeof( engineState.cameraPosition[0] ) ) { // koniec polaczenia
				close( connectionPool[i].s );
				connectionPool[i] = connectionPool.back();
				connectionPool.pop_back();
				cout << "zakonczylem polaczenie z jednym z klientow" << endl;
			}
		}

		// wysylamy dt (informacja o synchronizacji)
		for ( int i = 0; i < connectionPool.size(); i++ ) {
			auto cc = connectionPool[i];
			char buf[sizeof( dt )+sizeof( int )+sizeof( double )*3*( connectionPool.size()-1 )];
			char *p = buf;
			*( double * )p = dt;
			p+=sizeof( dt );
			*( int * )p = connectionPool.size()-1;
			p+= sizeof( int );
			for ( int j = 0; j < connectionPool.size(); j++ ) {
				if ( i != j ) {
					*( double * )p = connectionPool[j].position[0];
					p+= sizeof( double );
					*( double * )p = connectionPool[j].position[1];
					p+= sizeof( double );
					*( double * )p = connectionPool[j].position[2];
					p+= sizeof( double );
				}
			}
			auto ret = send( cc.s, buf, sizeof( dt )+sizeof( int )+sizeof( double )*3*( connectionPool.size()-1 ), MSG_NOSIGNAL );
		}

		auto sleepTime = microseconds( ( int )( 1000000.0*( ( ( double )frame )/fps - ( ( ( std::chrono::duration<double> )( system_clock::now() - t0 ) ).count() ) ) ) );
		if ( sleepTime.count() < 0 ) { // oj - nie zdazylismy wyswietlic klatki w zadanym czasie!!
			dropFrame = true;
			std::cout << "frame dropped\n";
		} else {
			dropFrame = false;
			sleep_for ( sleepTime );
		}
		auto now = system_clock::now();
		cdt = now - prevTime; // przyrost czasu w sekundach
		prevTime = now;
	}
	// zamykamy polaczenia
	for ( int i = 0; i < connectionPool.size(); i++ ) {
		close( connectionPool[i].s );
	}
	cout << "Zamykam serwer" << endl;
	close( sockfd );
}

int connectToServer( string srvaddr ) {
	int yes = 1;
	int clientsocket;
	struct sockaddr_in serv_addr;
	if( ( clientsocket = socket( AF_INET, SOCK_STREAM, 0 ) ) < 0 ) return -1;
	memset ( &serv_addr, 0, sizeof( serv_addr ) );
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons( 19191 );
	if( inet_pton( AF_INET, srvaddr.c_str(), &serv_addr.sin_addr )<=0 )return -2;
	if( connect( clientsocket, ( struct sockaddr * )&serv_addr, sizeof( serv_addr ) ) < 0 ) return -3;
	setsockopt( clientsocket, IPPROTO_TCP, TCP_NODELAY, ( char * ) &yes, sizeof( int ) );
	return clientsocket;
}

int main( int argc, char* argv[] ) {
	vector<thread> threads;
	int clientsocket;
	string serverAddr;
	double dt; // 60FPS


	if ( argc <= 1 ) {
		cout << "Serwer\n";
		threads.push_back( thread( serverThread ) );
		serverAddr = "127.0.0.1";// localhost
		while ( serverThreadState == 0 )
			sleep_for ( microseconds( 1 ) ); // TODO: Poprawic na sygnaly
	} else {
		serverAddr = argv[1];// localhost
	}

	if( ( clientsocket = connectToServer( serverAddr ) ) < 0 ) return -1;

	loadLevel();

	auto prevTime = std::chrono::system_clock::now();
	auto t0 = prevTime; // zero time
	std::chrono::duration<double> cdt; // frame time - calculated

	unsigned long long int frame = 0;

	bool dropFrame = false;
	while ( !engineState.quit ) {
		// stale framerate z mozliwoscia gubieniem klatek!
		if ( !dropFrame ) {
			doGraphics();
		}
		frame++;
		// odbieramy potwierdzenia

		doUserInteractions( dt );

		// tu synchronizacja odbywa sie za posrednictwem serwera - to co mozna dodac - mozliwosc gubienia klatek, w sytuacji, gdy nie nadazamy
		send( clientsocket, engineState.cameraPosition.data(), 3*sizeof( engineState.cameraPosition[0] ), MSG_NOSIGNAL );
		recv( clientsocket, &dt, sizeof( dt ), MSG_NOSIGNAL );
		int nclients = 0;
		recv( clientsocket, &nclients, sizeof( int ), MSG_NOSIGNAL );
		engineState.others.clear();
		for ( int i = 0; i < nclients; i++ ) {
			vector < double > p( 3 );
			recv( clientsocket, p.data(), sizeof( double )*3, MSG_NOSIGNAL );
			engineState.others.push_back( p );
		}
		doPhysics( dt );

	}
	close( clientsocket );
	serverThreadState = 0;

	cout << "zamykam polaczenia\n";
	for ( auto &t: threads ) t.join();
	cout << "OK\n";
	SDL_DestroyWindow( engineState.window );
	SDL_Quit();
	return 0;
}
