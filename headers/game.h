/*
 * game.h
 *
 *  Created on: Mar 29, 2018
 *      Author: TMK
 */

#ifndef SOURCE_HEADERS_GAME_H_
#define SOURCE_HEADERS_GAME_H_

#include <keyboardinput.h>
#include "animatedsprite.h"
#include <vector>
#include "player.h"
#include <memory>

class Graphics;

class Game{
public:

	Game();

	~Game();

	static Game& getInstance();

	void addNewSpriteToDraw(AnimatedSprite* sprite);

private:

	static Game* _instance;

	void gameLoop();

	void draw(Graphics &graphics);

	void update(float elapsedtime);

	std::vector<std::unique_ptr<AnimatedSprite>> _spritesToDraw;

	Player _player;

	KeyboardInput _input;

};



#endif /* SOURCE_HEADERS_GAME_H_ */
