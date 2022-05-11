#include <player.hpp>
#include <SDL.h>
#include <vector>
#include <tuple>
#include <map>
#include <cmath>
#include <functional>
#include <chrono>
#include <iostream>
#include <thread>
#include <stdexcept>

using namespace std;

Player::Player() {
		position = {0,0};
		speed = {0,0};
		acceleration = {0,0};
		std::tie(s_player, t_player) = spriteEngine.getResource("img/player.bmp");
	}


void Player::draw(double t) {
	SDL_Rect srcrect;
	SDL_Rect dstrect;
	int s = position[0]*10;
	srcrect = {((s&0x07)>>1)*32, ((speed[0] < 0)?1:0)*64, 32,64};
	auto pp = spriteEngine.toScreenCoord( position);
	dstrect = { pp[0], pp[1], (int)unitSize,(int)unitSize*2 };
		SDL_RenderCopy(spriteEngine.renderer, t_player, &srcrect, &dstrect);
/*		SDL_RenderCopyEx(game.renderer, game.t_player, &srcrect, &dstrect,
						 std::atan2(speed[1], abs(speed[0]))*180/M_PI,
						 NULL, //const SDL_Point*       center,
						 SDL_FLIP_NONE); // SDL_FLIP_HORIZONTAL 
						 * */
}

void Player::physics(GameMap &gameMap, double dt) {
	vector < double > footOffset = {+0.5, -1};
	auto prevPosition = position;
	position += speed*dt;
	speed += gravity*dt + acceleration*dt;
	speed -= speed*0.1*dt;
	
	// Kolizje -- wersja po punkcie
	if (gameMap.getCollision(position + footOffset) != 0) { // prosta kolizja
		if (gameMap.getCollision(prevPosition + footOffset) == 0) { // poprzednio bylo ok
			if (gameMap.getCollision({prevPosition[0]+footOffset[0], position[1]+footOffset[1]}) != 0) { // po osi Y
				speed[1] = 0;
				position[1] = prevPosition[1];
			}
			if (gameMap.getCollision({position[0]+0.5, prevPosition[1]-1}) != 0) { // po osi X
				speed[0] = 0;
				position[0] = prevPosition[0];
			}
		} else {
			while ((gameMap.getCollision(position+footOffset) != 0 ) && (position[1] < 1000)) { // naprawianie poczatkowej pozycji
				position += {0,0.1};
				speed = {0,0};
			}
		}
	}
	
	if (isOnGround(gameMap)) {
		speed -= speed*2.0*dt;
	}
}

bool Player::isOnGround(GameMap &gameMap) {
	vector < double >  footOffset = {+0.5, -1.05};
	
	if (gameMap.getCollision(position + footOffset) != 0) {
		return true;
	}
	return false;
}
