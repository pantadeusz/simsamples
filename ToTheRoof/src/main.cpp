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
#include "randutils.hpp"
#include <deque>
#include <cmath>
#include <algorithm> 

using namespace std;

int window_width, window_height; // uwaga zmienne globalne ; 

std::shared_ptr<SDL_Window> init_window(const int width = 350, const int height = 219)
{
	window_width = width;
	window_height = height;
	if (SDL_Init(SDL_INIT_VIDEO) != 0)
		throw std::runtime_error(SDL_GetError());

	SDL_Window *win = SDL_CreateWindow("ToTheRoof",
									   SDL_WINDOWPOS_UNDEFINED, 
									   SDL_WINDOWPOS_UNDEFINED,
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
	SDL_Renderer *ren = SDL_CreateRenderer(window.get(), -1, 
	SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
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
	SDL_Texture *tex = SDL_CreateTextureFromSurface(renderer.get(), bmp);
	std::shared_ptr<SDL_Texture> texture(tex, [](SDL_Texture *ptr) {
		SDL_DestroyTexture(ptr);
	});
	SDL_FreeSurface(bmp);
	return texture;
}

std::shared_ptr<SDL_Texture> load_png_texture(
	const std::shared_ptr<SDL_Renderer> renderer, 
	const std::string fname)
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

std::shared_ptr<SDL_Texture> create_texture(
	const std::shared_ptr<SDL_Renderer> renderer, 
	const int w, const int h)
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



class kwadrat
{
private:
	/* std::vector<std::array<unsigned char, 4>> palette {
    {255, 0, 0, 255},  // red
    {0, 255, 0, 255},  // green
	SDL_SetRenderDrawColor(renderer.get(), 0x89, 0x04, 0xB1, 0x00);// fioletowy
}; */
public:

const static int rect_w = 50;
const static int rect_h = 50;

///std::vector<vec_t>pole;

struct colour
{
	int r,g,b,a;
};

int kara;


vec_t position;
vec_t velocity;

enum type 
{
	plus = 1,
	minus = -1, 
	death = 0
};


colour c;

int t;


kwadrat(/* args */)
{

	// std::mt19937 rng;
   // rng.seed(std::random_device()());
    //std::uniform_int_distribution<std::mt19937::result_type> los(-1,1);
	//int n = los(-1,1);

	randutils::mt19937_rng rng;

	int n = rng.uniform(-1,1);

	 t = static_cast<type>(n);

	 

	if (t == -1) {
		 c.r = 255; 
		 c.b = 0; 
		 c.g = 0; 
		 c.a = 255;

		 kara = -1;
		
	}

	if (t == 1) {
		 c.r = 0; 
		 c.b = 255; 
		 c.g = 0; 
		 c.a = 255;
		 kara = 1;
		
	}

	if (t == 0) {
		 c.r = 0x89; 
		 c.b = 0x04; 
		 c.g = 0xB1; 
		 c.a = 0x00;
		 //kara = NULL;
	}
	//velocity = {0.0,0.0};
	
	
}
void oblicz_pole(std::vector <vec_t> &pole){
	pole.clear();

		vec_t start_point;
	 start_point[0] = position[0]-25;
	 start_point[1] = position[1]-25;
	
	for(int i = 0; i < 50; i++)
	{
		vec_t temp_point;

		temp_point[0]= start_point[0] + i;
		
		for(int j = 0; j < 50; j++)
		{

			 temp_point[1] = start_point[1] + j;
			 
			 pole.push_back(temp_point);
		}
		
	}//*/
	}

void draw(std::shared_ptr<SDL_Renderer> &r,
		const std::vector<int> collisions = {}
		) const {
	//	SDL_Rect rect = {(int)(position[0]-10),(int)(position[1]-15),rect_w,rect_h}; //oryginalna linijka
		SDL_Rect rect = {(int)(position[0]),(int)(position[1]),rect_w,rect_h};
		SDL_SetRenderDrawColor(r.get(),c.r,c.b,c.g,c.a);
		SDL_RenderDrawRect(r.get(), &rect);
		SDL_RenderFillRect(r.get(), &rect);
		
		int i =0;
		/*for (const auto &cp : collision_pts) {
			auto p = position + cp;
			SDL_Rect rect = {(int)(p[0]-3),(int)(p[1]-3),6,6};
			if ((collisions.size() > i) && collisions[i])
				SDL_SetRenderDrawColor(r.get(),c.r,c.b,c.g,c.a);
			else
				SDL_SetRenderDrawColor(r.get(),c.r,c.b,c.g,c.a);
			SDL_RenderDrawRect(r.get(), &rect);
			i++;
		}*/
	}

};

	







class player_t {
public:
	vec_t position;
	vec_t velocity;
	std::shared_ptr<SDL_Texture> player_image;

	int pkt;

	

	const static int rect_w = 20;
	const static int rect_h = 30;

	std::vector<vec_t>pole;
	
	std::vector<vec_t> collision_pts;
	std::vector<vec_t> collision_mod;
	void draw(std::shared_ptr<SDL_Renderer> &r,
		const std::vector<int> collisions = {}
		) const {
		//SDL_Rect rect = {(int)(position[0]-10),(int)(position[1]-15),20,30};  // oryginalna linijka
		SDL_Rect rect = {(int)(position[0]-10),(int)(position[1]-15),rect_w,rect_h}; 
		SDL_SetRenderDrawColor(r.get(),255,0,0,255);
		//SDL_RenderDrawRect(r.get(), &rect);
		SDL_RenderCopy(r.get(),player_image.get(),NULL,&rect);
		int i =0;
		for (const auto &cp : collision_pts) {
			auto p = position + cp;
			SDL_Rect rect = {(int)(p[0]-3),(int)(p[1]-3),6,6};
			if ((collisions.size() > i) && collisions[i])
				SDL_SetRenderDrawColor(r.get(),255,128,128,255);
			else
				SDL_SetRenderDrawColor(r.get(),0,0,255,255);
			//SDL_RenderDrawRect(r.get(), &rect);
			i++;
		}
	}
	
	std::vector<int> check_collision( std::vector < unsigned char> map_data,
			unsigned map_width,
			unsigned map_height) {
				std::vector<int> ret;
		for (const auto &cp : collision_pts) {
			auto p = position + cp;
			try{
			if(map_data.at(((int)(p[1])*map_width+(int)(p[0]))*4) == 255) {
				ret.push_back(true);
			} else {
				ret.push_back(false);
			}
			} catch (...) {
				ret.push_back(true);
			}
			//std::cout << "p:" << p[0] << ", " << p[1] << ": " << <<  std::endl;
			
		}
		return ret;
	}

	bool apply_collision(const std::vector<int> &collisions) {
		for (unsigned i = 0; i < collisions.size(); i++) {
			auto cmod = collision_mod[i];
			auto c = collisions[i];
			if (c) {
				position = position + cmod;
				return true;
			}
		}
		return false;
	}



	player_t( ) {

		pkt = 0;
		
		collision_pts = {
			{-5,-15},
			{-10, -10},
			{-10, 10},
			{-5,15},
			{5,15},
			{10, 10},
			{10, -10},
			{5,-15}
		};

		collision_mod = {
			{0,1},
			{1, 1},
			{1, -1},
			{0,-1},
			{0,-1},
			{-1, -1},
			{-1, 1},
			{0,1}
		};
		velocity = {0.0,0.0};

	}

	void oblicz_pole(vector <vec_t> &pole){
		pole.clear();

		vec_t start_point;
	 start_point[0] = position[0]-25;
	 start_point[1] = position[1]-25;
	
	for(int i = 0; i < 50; i++)
	{
		vec_t temp_point;

		temp_point[0]= start_point[0] + i;
		
		for(int j = 0; j < 50; j++)
		{

			 temp_point[1] = start_point[1] + j;
			 
			 pole.push_back(temp_point);
		}
		
	}
	}
};


int main()
{ // int argc, char **argv ) {
	using namespace std;
	auto window = init_window();
	auto renderer = init_renderer(window);
	int n = 0;
	



	vector < unsigned char> map_data;
	unsigned map_width;
	unsigned map_height;
	lodepng::decode(map_data, map_width, map_height, "data/map.png"); // load png image to vector (image)
	auto map_image =  load_png_texture(renderer, "data/map.png");
	

	player_t player;
	player.position[0] = 150;
	player.position[1] = 100;
	kwadrat kw;
	int sufit_w = window_width/kw.rect_w;
	std::deque<kwadrat> sufit;

	deque<kwadrat>temp_sufit;
	int ile_kw = 100000;

	for(int j=0; j< ile_kw; j++)
	{

	
		for(int i = 0; i < sufit_w; i++)
		{
			kwadrat kw;
			kw.position[0]+=(50 * i );
			sufit.push_back(kw);
		} 

		
	
	}

	cout << "sufit ma długość : " << sufit.size() << "\n";
	

	cout << kw.t << " " << kw.c.b << " "<< kw.c.g << " "<< kw.c.r << " "<< kw.c.a << "\n";

	player.player_image =  load_png_texture(renderer, "data/bullet.png");

	
	
	int can_jump =  0;
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
			}
		}
		const Uint8 *kstate = SDL_GetKeyboardState(NULL);


		if ((kstate[SDL_SCANCODE_LEFT])&&(player.position[0]>=10)) player.position[0]--;
		if ((kstate[SDL_SCANCODE_RIGHT])&& (player.position[0]<=320)) player.position[0]++; // na półce p = (291,34.5)
		if (kstate[SDL_SCANCODE_UP]) {
			if (can_jump>0) {
				can_jump = 0;
				player.position[1]--;
				player.velocity[1] = -13;
				//player.velocity[0] = -1;
				n++;
				
				
				
			}
		}
//		if (kstate[SDL_SCANCODE_DOWN]) player.position[1]++;

		player.position = player.position + player.velocity;
		player.velocity = player.velocity + vec_t{0.0, 0.5};
		if (can_jump > 0) can_jump--;

		auto collisions = player.check_collision(map_data, map_width, map_height);
		bool colided;
		auto collisions_check = collisions;
		do {
			colided = player.apply_collision(collisions_check);
			if (colided) {
				player.velocity = {0,0};
				collisions_check = player.check_collision(map_data, map_width, map_height);
			}
		} while (colided);

		if ((collisions[3] == 1) || (collisions[4] == 1)) {
			can_jump = 3;
		}
		SDL_RenderClear(renderer.get());
		
		SDL_RenderCopy(renderer.get(), map_image.get(), NULL, NULL);
		

		// ładowanie textury nie powinno być co klatke czyli nie powinoo być w pętli
		

		// poczatek z tutoriala

		//SDL_RenderDrawRect(renderer.get(),&r);
		//SDL_SetRenderDrawColor(renderer.get(), kw.c.r, kw.c.g, kw.c.b, kw.c.a);
		//SDL_RenderFillRect(renderer.get(), &r);
		//kw.move(20);


/*
		//SDL_SetRenderTarget(renderer.get(), texture_r);
               SDL_RenderDrawRect(renderer.get(),&r);
              // SDL_SetRenderDrawColor(renderer.get(), 0xFF, 0x00, 0x00, 0x00);
			   SDL_SetRenderDrawColor(renderer.get(), 0x89, 0x04, 0xB1, 0x00);
               SDL_RenderFillRect(renderer.get(), &r);

			   //8904B1 fioletowy html
		// koniec z tutoriala */
		
		
		//cout << sufit.size() << "\n";
		//deque<kwadrat>temp_sufit;
		bool zderzenie = false;
		for(int i = 0; i < sufit_w*3; i++)
		{
			
			
			
			kwadrat kw = sufit[i+n];
			//kw.draw(renderer, collisions);
			temp_sufit.push_back(kw);

			
		}
		kwadrat hit_kw;

		for(int i = 0; i<temp_sufit.size();i++)
		{
			kwadrat kw = temp_sufit[i];

			

			// obsługa zderzenia 

			
				

				vector<vec_t> pole_kw;
				vector<vec_t> pole_pl = {};
				//temp_sufit[i].oblicz_pole(pole_kw);
				kw.oblicz_pole(pole_kw);

				player.oblicz_pole(pole_pl);
			
			
				

				
				for(int j = 0; j < kw.rect_w*kw.rect_h; j++)
				{
					 
					for(int k = 0; k < player.rect_w*player.rect_h; k++)
					{
						
						if ((pole_kw[j][0]==pole_pl[k][0])&&(pole_kw[j][1]==pole_pl[k][1])) {
							zderzenie = true;
							hit_kw = kw;
							break;
						}

						
						
					}

					if(zderzenie)break;
					
					
				}

				
				//pole_kw.clear();
				//pole_pl.clear();
			
			

			
			



		}
		if (zderzenie) {
			if (hit_kw.t == 0) {
					cout << "koniec gry dotkołeś purpury zdobyłeś : " << player.pkt << "\n";
					player.pkt = 0 ;
				}
				
				else
				{
					player.pkt += hit_kw.kara;

					cout << "punkty gracza: " << player.pkt << "\n"; 
				}

				player.position[0];

				//pole_kw.clear();
				//pole_pl.clear();

				zderzenie = false;
		}

		
		
		
		
		
/*
		kwadrat new_kw; 
		sufit.pop_front();
		sufit.push_back(new_kw);*/
		
		//kw.draw(renderer, collisions);
		
		
		

		std::cout << "p: " << player.position[0] << " " << player.position[1] <<"         \r";
		

		
		for(int  i = 0; i < temp_sufit.size(); i++)
		{
			player.draw(renderer, collisions);
			temp_sufit[i].draw(renderer, collisions);
		}

		std::cout.flush(); 
		
		SDL_RenderPresent(renderer.get());
		


		temp_sufit.clear();

		SDL_Delay(10);
		
	
	
}
return 0;
}
