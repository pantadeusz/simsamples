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


map<SDL_Keycode,function < void (SDL_Event &) > > keyDownHandlers;
map<SDL_Keycode,function < void (SDL_Event &) > > keyUpHandlers;

template < typename T > 
vector < T > operator +(const vector < T > &a, const vector < T > &b) {
	vector < T > r(a.size());
	for (int i = 0; i < a.size(); i++) r[i] = a[i] + b[i];
	return r;
}
template < typename T > 
vector < T > operator -(const vector < T > &a, const vector < T > &b) {
	vector < T > r(a.size());
	for (int i = 0; i < a.size(); i++) r[i] = a[i] - b[i];
	return r;
}
template < typename T > 
vector < T > &operator +=(vector < T > &a, const vector < T > &b) {
	for (int i = 0; i < a.size(); i++) a[i] += b[i];
	return a;
}
template < typename T > 
vector < T > &operator *=(vector < T > &a, const vector < T > &b) {
	for (int i = 0; i < a.size(); i++) a[i] *= b[i];
	return a;
}
template < typename T > 
vector < T > &operator *=(vector < T > &a, const T  &b) {
	for (int i = 0; i < a.size(); i++) a[i] *= b;
	return a;
}

template < typename T > 
T operator ~(const vector < T > &a) {
	T r;
	for (int i = 0; i < a.size(); i++) r += a[i]*a[i];
	return sqrt(r);
}
// zwraca procent odleglosci miedzy punktami x, y
// a---p---b
double getT(const vector < double > &a, const vector < double > &b, const vector < double > &p) {
	double l = ~(b-a);
	if (l == 0) return 0.5;
	double t = (~(p-a))/l;
}

vector < double > intersect(const vector < double > &p1,
        const vector < double > &p2,
        const vector < double > &p3,
        const vector < double > &p4) {
double x1 = p1[0], x2 = p2[0], x3 = p3[0], x4 = p4[0];
double y1 = p1[1], y2 = p2[1], y3 = p3[1], y4 = p4[1];
 
double d = (x1 - x2) * (y3 - y4) - (y1 - y2) * (x3 - x4);
if (d == 0) return {p1[0], p1[1]};
 
double pre = (x1*y2 - y1*x2), post = (x3*y4 - y3*x4);
double x = ( pre * (x3 - x4) - (x1 - x2) * post ) / d;
double y = ( pre * (y3 - y4) - (y1 - y2) * post ) / d;
/* 
// Check if the x and y coordinates are within both lines
if ( x < min(x1, x2) || x > max(x1, x2) ||
x < min(x3, x4) || x > max(x3, x4) ) return NULL;
if ( y < min(y1, y2) || y > max(y1, y2) ||
y < min(y3, y4) || y > max(y3, y4) ) return NULL;
*/
return {x,y};
}

vector < double > intersect2(
        const vector < double > &p1,
        const vector < double > &p2,
        const vector < double > &p3,
        const vector < double > &p4
        ) {
	double x1 = p1[0],
           y1 = p1[1],
           x2 = p2[0],
           y2 = p2[1],
           x3 = p3[0],
           y3 = p3[1],
           x4 = p4[0],
           y4 = p4[1];
   return {
	   ((x1*y2-y1*x2)*(x3-x4)-(x1-x2)*(x3*y4-y3*x4))/
		((x1-x2)*(y3-y4)-(y1-y2)*(x3-x4)),
		((x1*y2-y1*x2)*(y3-y4)-(y1-y2)*(x3*y4-y3*x4))/
		((x1-x2)*(y3-y4)-(y1-y2)*(x3-x4))
  };
}

void drawTriangle(SDL_Renderer *renderer, const vector < vector < double > > p, const vector < vector < double > > c) {
	vector < vector < double > > psort = p;
	vector < vector < double > > csort = c;
	
	    // sortowanie po y:
    if (psort[0][1] > psort[1][1]) {
		auto tmp = psort[0]; psort[0] = psort[1]; psort[1] = tmp;
		auto tmp2 = csort[0]; csort[0] = csort[1]; csort[1] = tmp2;
	}
    if (psort[1][1] > psort[2][1]) {
		auto tmp = psort[1]; psort[1] = psort[2]; psort[2] = tmp;
		auto tmp2 = csort[1]; csort[1] = csort[2]; csort[2] = tmp2;
	}
    if (psort[0][1] > psort[1][1]) {
		auto tmp = psort[0]; psort[0] = psort[1]; psort[1] = tmp;
		auto tmp2 = csort[0]; csort[0] = csort[1]; csort[1] = tmp2;
	} 

	double y1 = psort[0][1];
    double y2 = psort[1][1];
    double y3 = psort[2][1];

    double x1 = psort[0][0];
    double x2 = psort[1][0];
    double x3 = psort[2][0];

    // Bounding rectangle
    int minx = (int)std::min(std::min(x1, x2), x3);
    int maxx = (int)std::max(std::max(x1, x2), x3);
    int miny = (int)std::min(std::min(y1, y2), y3);
    int maxy = (int)std::max(std::max(y1, y2), y3);


    // Scan through bounding rectangle
    for (double y = miny; y < maxy; y++) {
        for (double x = minx; x < maxx; x++) {
            if( (((x1 - x2) * (y - y1) - (y1 - y2) * (x - x1)) < 0) &&
             (((x2 - x3) * (y - y2) - (y2 - y3) * (x - x2)) < 0) &&
             (((x3 - x1) * (y - y3) - (y3 - y1) * (x - x3)) < 0)) {
				SDL_SetRenderDrawColor(renderer, 
					//cl[0]*255.0, 
					//cl[1]*255.0,
					//cl[2]*255.0, 
					123,123,123,
					255);
				SDL_RenderDrawPoint(renderer, x, y); 
				 vector < double > cl = {0,0,0};
					 
				 for (int i = 0; i < 3; i++) {
					 auto ip = intersect(p[1], p[2], p[0], {x,y});
					 double ec = c[1][i]*getT(p[1],p[2],ip)
						+c[2][i]*getT(p[2],p[1],ip);
					 cl[i] = ec;
					 //cl[i] = c[0][i]*getT(p[0],ip,{x,y})
						//+ec*getT(ip,p[0],{x,y});
				 }
				 SDL_SetRenderDrawColor(renderer, 
					cl[0]*255.0, 
					cl[1]*255.0,
					cl[1]*255.0, 
					
					255);
				SDL_RenderDrawPoint(renderer, x, y);
            }
        }
    }
}

int main( int argc, char* args[] ) { 
	bool finishCondition = false;
	

    SDL_Init( SDL_INIT_EVERYTHING ); 
	SDL_Window *window;
	SDL_Renderer *renderer;
	SDL_CreateWindowAndRenderer(800, 600, 0, &window, &renderer);
    vector <double > p(2); // pozycja
    vector <double > dp(2); // predkosc
    vector <double > ddp(2); // przyspieszenie
    p[0] = 0; p[1] = 0;
    dp[0] = 0; dp[1] = 0;
    ddp[0] = 0; ddp[1] = 0;
    
	keyDownHandlers[SDLK_ESCAPE] = [&](SDL_Event &e){finishCondition=true;};
    
    while (!finishCondition) {
		// grafika
		// glVertex3f(p[0], p[1], 0.0);
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
		SDL_RenderClear(renderer);
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
		for (double px = -0.01; px < 0.01; px+= 0.001) 
		for (double py = -0.01; py < 0.01; py+= 0.001) 
		SDL_RenderDrawPoint(renderer, (p[0]+px)*400.0 + 400.0, -(p[1]+py)*300.0 + 300.0); //Renders on middle of screen.
		//SDL_RenderDrawPoint(renderer, p[0]*400.0 + 400.0, -p[1]*300.0 + 300.0); //Renders on middle of screen.
		/*{
			vector < vector < double > > tp = { {130,11}, {150,50}, {10,150} };
			vector < vector < double > > tc = { {0,1,0},{1,0,0}, {0,0,1} };
			drawTriangle(renderer,  tp, tc);
		}*/
		{
			vector < vector < double > > tp = { {250,50}, {230,120},  {110,150} };
			vector < vector < double > > tc = { {0,1,0},{1,0,0}, {0,0,1} };
			drawTriangle(renderer,  tp, tc);
		}
		
		SDL_RenderPresent(renderer);	
        // delay
        sleep_for (milliseconds(33));

        // zdarzenia
		SDL_Event e;
		while( SDL_PollEvent( &e ) != 0 ) { 
			if( e.type == SDL_QUIT ) { 
				finishCondition = true;
			} else if (e.type == SDL_KEYDOWN) {
				if (keyDownHandlers.count(e.key.keysym.sym) > 0) keyDownHandlers[e.key.keysym.sym](e);
			} else if (e.type == SDL_KEYUP) {
				if (keyUpHandlers.count(e.key.keysym.sym) > 0) keyUpHandlers[e.key.keysym.sym](e);
			}
		}
		
		// sterowanie
		const Uint8 *state = SDL_GetKeyboardState(NULL);
		// zerujemy aktuatory
		ddp[0] = 0;
		ddp[1] = 0;
        // sprawdzamy klawisze
		if (state[SDL_SCANCODE_LEFT]) ddp[0] = -0.01;
		if (state[SDL_SCANCODE_RIGHT]) ddp[0] = 0.01;
		if (state[SDL_SCANCODE_UP]) ddp[1] = 0.01;
		if (state[SDL_SCANCODE_DOWN]) ddp[1] = -0.01;

		//fizyka
		dp += ddp;
		p += dp;
		dp *= 0.9;

		
	}
   
    SDL_DestroyWindow( window );
    SDL_Quit(); 
    return 0; 
}
