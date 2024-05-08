
#include <SDL.h>
#include <SDL_timer.h>
#include <cstdint>
#include <vector>
#include <memory>
#include <stdexcept>
#include <iostream>


/**
 * @brief Generates texture from 32 bit pixel data with transparrency
 * 
 * @param renderer renderer for texture
 * @param pixels pixel data written by rows
 */
std::shared_ptr<SDL_Texture> generate_texture_from_pixels(SDL_Renderer * renderer, std::vector<std::uint32_t> &pixels, int width, int height) {
        SDL_Surface *surface = SDL_CreateRGBSurfaceFrom(pixels.data(),
                                      width,
                                      height,
                                      32,
                                      width*4, // pitch
                                      0x0ff, //Rmask,
                                      0x0ff00, //Gmask,
                                      0x0ff0000, //Bmask,
                                      0x0ff000000 //Amask
                                      );
        if (!surface) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't create surface from image: %s", SDL_GetError());
            throw std::logic_error(SDL_GetError()) ;
        }
        SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
        if (!texture) {
            // SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't create texture from surface: %s", SDL_GetError());
            // return 3;
            throw std::logic_error(SDL_GetError()) ;
        }
        SDL_FreeSurface(surface);
        
        return std::shared_ptr<SDL_Texture> (texture, [](auto *p){SDL_DestroyTexture(p);});
}

int main(int argc, char *argv[])
{
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Surface *surface;
    SDL_Texture *texture;
    SDL_Event event;

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't initialize SDL: %s", SDL_GetError());
        return 3;
    }

    if (SDL_CreateWindowAndRenderer(320, 240, SDL_WINDOW_RESIZABLE, &window, &renderer)) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't create window and renderer: %s", SDL_GetError());
        return 3;
    }


    std::vector<std::uint32_t> pixels(16*16);
    while (1) {
        SDL_PollEvent(&event);
        if (event.type == SDL_QUIT) {
            break;
        }
        SDL_SetRenderDrawColor(renderer, 0x00, 0xff, 0x00, 0x00);
        SDL_RenderClear(renderer);
        auto j = SDL_GetTicks();
        for (int i = 0; i < 16*16; i++) {
            pixels[i] = ((i + ((i / 16) <<8) + ((i^ 7)<<16)) ^ (j & 0x0ffffff)) + 0x0ff000000;
        }
        auto texture = generate_texture_from_pixels(renderer, pixels, 16, 16);
        SDL_RenderCopy(renderer, texture.get(), NULL, NULL);

        SDL_RenderPresent(renderer);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    SDL_Quit();

    return 0;
}