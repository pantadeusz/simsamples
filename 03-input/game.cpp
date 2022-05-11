// SOURCE: https://wiki.libsdl.org/SDL_CreateWindowAndRenderer

#include <SDL.h>
#include <iostream>

int main(int argc, char *argv[])
{
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Event event;
    SDL_GameController *ctrl;
    SDL_Joystick *joy;
    int i;

    if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't initialize SDL: %s", SDL_GetError());
        return 3;
    }

    if (SDL_CreateWindowAndRenderer(320, 240, SDL_WINDOW_RESIZABLE, &window, &renderer)) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't create window and renderer: %s", SDL_GetError());
        return 3;
    }

    for(i = 0; i < SDL_NumJoysticks(); ++i) {
        if (SDL_IsGameController(i)) {
            std::cout << i << ": " << SDL_GameControllerNameForIndex(i) << std::endl;
            ctrl = SDL_GameControllerOpen(i);
            joy = SDL_GameControllerGetJoystick(ctrl);
            break;
        } else {
            std::cout << "Incopatible controller at index " << i << std::endl;
        }
    }

    while (1) {
        SDL_PollEvent(&event);
        if (event.type == SDL_QUIT) {
            break;
        }

        int16_t xVal = SDL_GameControllerGetAxis(ctrl, SDL_CONTROLLER_AXIS_LEFTX);
        int16_t yVal = SDL_GameControllerGetAxis(ctrl, SDL_CONTROLLER_AXIS_LEFTY);

        SDL_SetRenderDrawColor(renderer, abs(xVal) >> 8 , abs(yVal)  >> 8, 0, 0x00);
        SDL_RenderClear(renderer);
        SDL_RenderPresent(renderer);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    SDL_Quit();

    return 0;
}