/**
 * This is the simple hello world for SDL2.
 *
 * TODO:
 * 

    1. Load Bitmap

	SDL_Surface *bmp = SDL_LoadBMP( fname.c_str() );
	SDL_FreeSurface( ptr );

    2. Create texture
    
	SDL_Texture *tex = SDL_CreateTextureFromSurface( renderer, bitmap );
	SDL_DestroyTexture( tex );

    3. Load texture using lodepng

  	unsigned error = lodepng::decode(image, width, height, fname);
	if(error) throw std::runtime_error( lodepng_error_text(error));

	SDL_Surface* bitmap = SDL_CreateRGBSurfaceFrom( ... )

	4. draw texture to renderer
	
	SDL_RenderCopy( renderer.get(), game_texture_1.get(), NULL, &dstrect );

    4. collisions

	I will show 8 point collision

 */

#include <SDL2/SDL.h>
#include <lodepng.h>
#include <stdexcept>
#include <memory>
#include <string>
#include <set>
#include <tuple>
#include <iostream>
#include <cstdint>
#include <vector>
#include <random>

std::shared_ptr<SDL_Window> init_window(const int width = 320, const int height = 200)
{
	if (SDL_Init(SDL_INIT_VIDEO) != 0)
		throw std::runtime_error(SDL_GetError());

	SDL_Window *win = SDL_CreateWindow("Witaj w Swiecie",
									   SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
									   width, height, SDL_WINDOW_SHOWN);
	if (win == nullptr)
		throw std::runtime_error(SDL_GetError());
	std::shared_ptr<SDL_Window> window(win, [](SDL_Window *ptr) {
		SDL_DestroyWindow(ptr);
	});
	return window;
}

std::shared_ptr<SDL_Renderer> init_renderer(std::shared_ptr<SDL_Window> window)
{
	SDL_Renderer *ren = SDL_CreateRenderer(window.get(), -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if (ren == nullptr)
		throw std::runtime_error(SDL_GetError());
	std::shared_ptr<SDL_Renderer> renderer(ren, [](SDL_Renderer *ptr) {
		SDL_DestroyRenderer(ptr);
	});
	return renderer;
}

std::shared_ptr<SDL_Texture> load_texture(const std::shared_ptr<SDL_Renderer> renderer, const std::string fname)
{
	SDL_Surface *bmp = SDL_LoadBMP(fname.c_str());
	if (bmp == nullptr)
		throw std::runtime_error(SDL_GetError());
	std::shared_ptr<SDL_Surface> bitmap(bmp, [](SDL_Surface *ptr) {
		SDL_FreeSurface(ptr);
	});

	SDL_Texture *tex = SDL_CreateTextureFromSurface(renderer.get(), bitmap.get());
	if (tex == nullptr)
		throw std::runtime_error(SDL_GetError());
	std::shared_ptr<SDL_Texture> texture(tex, [](SDL_Texture *ptr) {
		SDL_DestroyTexture(ptr);
	});
	return texture;
}

std::shared_ptr<SDL_Texture> load_png_texture(const std::shared_ptr<SDL_Renderer> renderer, const std::string fname)
{
	std::vector<unsigned char> image;
	unsigned width, height;
	unsigned error = lodepng::decode(image, width, height, fname); // load png image to vector (image)
	if (error)
		throw std::runtime_error(lodepng_error_text(error));

	SDL_Texture *tex = SDL_CreateTexture(renderer.get(), SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STATIC, width, height);
	if (tex == nullptr)
		throw std::runtime_error(SDL_GetError());
	std::shared_ptr<SDL_Texture> texture(tex, [](SDL_Texture *ptr) { SDL_DestroyTexture(ptr); });
	SDL_SetTextureBlendMode(texture.get(), SDL_BLENDMODE_BLEND); // obslugujemy polprzezroczystosc
	SDL_UpdateTexture(texture.get(), NULL, image.data(), width * sizeof(uint32_t));
	return texture;
}

std::shared_ptr<SDL_Texture> create_texture(const std::shared_ptr<SDL_Renderer> renderer, const int w, const int h)
{
	SDL_Texture *tex = SDL_CreateTexture(renderer.get(), SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, w, h);
	if (tex == nullptr)
		throw std::runtime_error(SDL_GetError());
	std::shared_ptr<SDL_Texture> texture(tex, [](SDL_Texture *ptr) { SDL_DestroyTexture(ptr); });
	return texture;
}

using vec_t = std::array<double, 2>;

vec_t operator-(const vec_t &a, const vec_t &b)
{
	return {a[0] - b[0], a[1] - b[1]};
}
vec_t operator+(const vec_t &a, const vec_t &b)
{
	return {a[0] + b[0], a[1] + b[1]};
}
vec_t operator*(const vec_t &a, const vec_t &b)
{
	return {a[0] * b[0], a[1] * b[1]};
}
vec_t operator*(const vec_t &a, const double b)
{
	return {a[0] * b, a[1] * b};
}
vec_t operator/(const vec_t &a, const double b)
{
	return {a[0] / b, a[1] / b};
}
double length(const vec_t &d_)
{
	auto d = d_ * d_;
	return std::sqrt(d[0] + d[1]);
}

void draw_player(SDL_Renderer *renderer, vec_t player_p, const std::array<vec_t, 8> &player_hit_points, const std::array<int,8> &player_hit_points_touch = {0,0,0,0,0,0,0,0})
{
	SDL_Rect rect = {player_p[0] - 10, player_p[1] - 15, 20, 30};
	SDL_SetRenderDrawColor(renderer, 255, 128, 128, 255);
	SDL_RenderDrawRect(renderer,&rect);

	for (size_t i = 0; i < player_hit_points.size(); i++) {
		if (player_hit_points_touch[i] == 0) SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
		else SDL_SetRenderDrawColor(renderer, 0, 128, 128, 255);
		auto p = player_hit_points[i] + player_p;
		SDL_Rect rect = {p[0]-3, p[1] - 3, 6, 6};
		SDL_RenderDrawRect(renderer,&rect);
	}
}

int main()
{ // int argc, char **argv ) {

	auto window = init_window();
	auto renderer = init_renderer(window);

	// we will be rendering data onto this:
	auto game_texture = load_png_texture(renderer, "data/map.png");

	std::vector<unsigned char> game_map;
	unsigned game_width, game_height;
	unsigned error = lodepng::decode(game_map, game_width, game_height, "data/map.png"); // load png image to vector (image)
	if (error)
		throw std::runtime_error(lodepng_error_text(error));

	std::random_device rd;
	std::mt19937 mt(rd());
	std::uniform_real_distribution<double> dist(1.0, 10.0);

	vec_t player_p = {100, 100};
	vec_t player_target_p = {100, 100};
	std::array<vec_t, 8> player_hit_points;
	std::array<int, 8> player_hit_points_touch;
	player_hit_points[1] = {-10, 10};
	player_hit_points[2] = {-5, 15};
	player_hit_points[3] = {5, 15};
	player_hit_points[4] = {10, 10};

	player_hit_points[5] = {10, -10};
	player_hit_points[6] = {5, -15};
	player_hit_points[7] = {-5, -15};
	player_hit_points[0] = {-10, -10};


	std::array<vec_t, 8> player_hit_points_dir_to_move;
	player_hit_points_dir_to_move[1] = {1, -1};
	player_hit_points_dir_to_move[2] = {0, -1};
	player_hit_points_dir_to_move[3] = {0, -1};
	player_hit_points_dir_to_move[4] = {-1, -1};

	player_hit_points_dir_to_move[5] = {-1, 1};
	player_hit_points_dir_to_move[6] = {0, 1};
	player_hit_points_dir_to_move[7] = {0, 1};
	player_hit_points_dir_to_move[0] = {1, 1};

	for (bool game_active = true; game_active;)
	{
		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			switch (event.type)
			{
			case SDL_QUIT:
				game_active = false;
				break;
			case SDL_MOUSEMOTION:
				player_target_p[0] = event.motion.x;
				player_target_p[1] = event.motion.y;
				break;
			}
		}


		auto next_player_p = player_p + (player_target_p - player_p) / 50;
		bool collision = false;
		do {
			collision = false;
			for (size_t i = 0; i < player_hit_points.size(); i++) {
				auto p = player_hit_points[i] + next_player_p;
				player_hit_points_touch[i] = 0;
				try {
					if (game_map.at(4*((int)(p[1])*game_width+(int)(p[0]))) > 2 ) {
						player_hit_points_touch[i] = 1;
					}
				} catch (...) {

				}
			}

			{
				for (size_t i = 0; i < player_hit_points.size(); i++) {
					if (player_hit_points_touch[i] == 1) {
						collision = true;
						next_player_p = next_player_p + player_hit_points_dir_to_move[i];
					}
				}
				
			}
		} while (collision);
		player_p = next_player_p;

		SDL_RenderClear(renderer.get());
		SDL_RenderCopy(renderer.get(), game_texture.get(), NULL, NULL);

		draw_player(renderer.get(), player_p, player_hit_points,player_hit_points_touch);

		SDL_RenderPresent(renderer.get());
		SDL_Delay(10);
	}
	return 0;
}
