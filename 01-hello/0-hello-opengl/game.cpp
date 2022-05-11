#include <SDL.h>
#include <GL/gl.h>

int main( int argc, char* args[] ) { 
    SDL_Init( SDL_INIT_EVERYTHING ); 
    auto window = SDL_CreateWindow( "Okienko SDL", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 480, SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL ); 
    SDL_GL_CreateContext(window);
    
    glClear(GL_COLOR_BUFFER_BIT);
    glBegin(GL_POLYGON);
        glVertex3f(0.0, 0.0, 0.0);
        glVertex3f(0.5, 0.0, 0.0);
        glVertex3f(0.5, 1, 0.0);
        glVertex3f(0.0, 0.5, 0.0);
    glEnd();
    //glFlush();
    
    SDL_GL_SwapWindow(window);
    SDL_Delay(2000);
    SDL_DestroyWindow( window );
    SDL_Quit(); 
    return 0; 
}
