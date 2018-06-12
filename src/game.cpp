/*
 * game.cpp
 *
 *  Created on: Mar 29, 2018
 *      Author: TMK
 */

#include <rat.h>
#include <SDL2/SDL.h>

#include "game.h"
#include "graphics.h"
#include <iostream>
#include "animatedsprite.h"
#include "backgroundsectorlibraryhandler.h"
#include <ctime>
#include "portal.h"
#include "globals.h"
#include "menubackground.h"
#include "pausemenu.h"
#include "mainmenu.h"
#include "backgroundblock.h"
#include "quadtree.h"
#include "hudplayerhp.h"
#include <SDL2/SDL_ttf.h>



namespace {
	const int MINFPS = 33;
	const float MAX_FRAME_TIME = 1000/MINFPS;
	const int MAXFPS = 60;
	const float MIN_FRAME_TIME = 1000/MAXFPS;
}

Game* Game::_instance = nullptr;

Game::Game():
	_keyboardInput(),
	_gamepadInputPlayer1(),
	_gamepadInputPlayer2(),
	_mapWidth(0),
	_mapHeight(0),
	_numberBlocksLine(background_blocks_constants::INITIAL_NUMBER_BLOCKS_LINE),
	_numberBlocksColumn(background_blocks_constants::INITIAL_NUMBER_BLOCKS_COLUMN),
	_quitFlag(false),
	_backgroundSectorHandler(),
	_graphicsAssociated(nullptr),
	_menuToReplaceInStack(nullptr),
	_showFpsFlag(false),
	_fps(0),
	_vSyncFlag(true),
	_minFrameTime(0){
	_instance = this;
	SDL_Init(SDL_INIT_EVERYTHING);
	if(TTF_Init() != 0){
		std::cout << "TTF_Init error: " << TTF_GetError() << std::endl;
	}
	srand(time(NULL));
	this->gameLoop();
}

Game::~Game(){
	this->_spritesToDraw.clear();
	this->_hudElements.clear();
}

Game& Game::getInstance(){
	if(_instance == nullptr){
		_instance = new Game();
	}
	return *_instance;
}


void Game::gameLoop(){

	this->_gamepadInputPlayer1.initGamepad();

	this->_gamepadInputPlayer2.initGamepad();

	Graphics graphics = Graphics(*this);

	this->_graphicsAssociated = &graphics;

	this->setupBackgroundBlocks(graphics, this->_numberBlocksLine , this->_numberBlocksColumn);

	SDL_Event event;

	this->_textCaches = TextCaches(graphics);

	int LAST_UPDATE_TIME = SDL_GetTicks();

	this->_player = Player(graphics, player_constants::PLAYER_START_X, player_constants::PLAYER_START_Y);
	this->_sharedPtrPlayer.reset(&this->_player);

	this->_vaccumcleaner = VacuumCleaner(graphics, this->_player);
	this->_sharedPtrVaccumCleaner.reset(&this->_vaccumcleaner);
	this->_sharedPtrVaccumCone.reset(&this->_vaccumcleaner.vCone);

	float fpsTimer = 0;

	float fps = 0;

	int fpsSampleCounter = 0;

	this->createNewPseudoRandomBlocksVector(background_blocks_constants::NUMBER_SECTORS_LINE, background_blocks_constants::NUMBER_SECTORS_COLUMN);

	this->update(MAX_FRAME_TIME);

	this->draw(graphics);

	this->_menuStack.emplace(new MainMenu(graphics, this->_keyboardInput, this->_gamepadInputPlayer1));

	while(true){


		const int CURRENT_TIME_MS = SDL_GetTicks();
		float ELAPSED_TIME_MS = CURRENT_TIME_MS - LAST_UPDATE_TIME;

		if(this->_vSyncFlag){

			this->_minFrameTime = (this->_graphicsAssociated->displayInfo.refresh_rate != 0)? 1000/this->_graphicsAssociated->displayInfo.refresh_rate : 1000/60;
			if(ELAPSED_TIME_MS < this->_minFrameTime){
				//arrumar isso aqui depois
				SDL_Delay(std::max(0.0f,(this->_minFrameTime - ELAPSED_TIME_MS)));
				const int CURRENT_TIME_MS = SDL_GetTicks();
				ELAPSED_TIME_MS = CURRENT_TIME_MS - LAST_UPDATE_TIME;
			}
		}

		LAST_UPDATE_TIME = CURRENT_TIME_MS;

		fpsTimer += ELAPSED_TIME_MS;
		fps += 1000/ELAPSED_TIME_MS;
		fpsSampleCounter++;

		if(fpsTimer > 1000){

			std::cout << "quantidade de objetos no mapa:   " << this->_spritesToDraw.size() << std::endl;
			std::cout << "fps depois da corre��o:   " << fps/fpsSampleCounter << std::endl;
			this->_fps = fps/fpsSampleCounter;
			fpsTimer = 0;
			fps = 0;
			fpsSampleCounter = 0;
		}


//		std::cout << " ======= new frame on game loop ======== " << std::endl;

		graphics.updateDisplayInfo();

		if(this->_quitFlag){
			return;
		}

		this->_keyboardInput.beginNewFrame();
		this->_gamepadInputPlayer1.beginNewFrame();
		this->_gamepadInputPlayer2.beginNewFrame();

		while(SDL_PollEvent(&event)){

			if(event.type == SDL_CONTROLLERDEVICEADDED){
				this->_gamepadInputPlayer1.initGamepad();
				if(event.cdevice.which != this->_gamepadInputPlayer1.getGamepadId()){
					this->_gamepadInputPlayer2.initGamepad();
				}
			}

			if(event.type == SDL_CONTROLLERDEVICEREMOVED){
				if(event.cdevice.which == this->_gamepadInputPlayer1.getGamepadId()){
					this->_gamepadInputPlayer1.closeGamepad();
				}
				if(event.cdevice.which == this->_gamepadInputPlayer2.getGamepadId()){
					this->_gamepadInputPlayer2.closeGamepad();
				}

			}

			if(event.type == SDL_CONTROLLERBUTTONDOWN){

				if(event.cdevice.which == this->_gamepadInputPlayer1.getGamepadId()){
					this->_gamepadInputPlayer1.buttonDownEvent(event);
				}
				if(event.cdevice.which == this->_gamepadInputPlayer2.getGamepadId()){
					this->_gamepadInputPlayer2.buttonDownEvent(event);
				}

			}

			if(event.type == SDL_CONTROLLERBUTTONUP){

				if(event.cdevice.which == this->_gamepadInputPlayer1.getGamepadId()){
					this->_gamepadInputPlayer1.buttonUpEvent(event);
				}
				if(event.cdevice.which == this->_gamepadInputPlayer2.getGamepadId()){
					this->_gamepadInputPlayer2.buttonUpEvent(event);
				}

			}

			if(event.type == SDL_KEYDOWN){
				if(event.key.repeat == 0){
					this->_keyboardInput.keyDownEvent(event);
				}
			}
			else if(event.type == SDL_KEYUP){
				this->_keyboardInput.keyUpEvent(event);
			}
			if(event.type == SDL_QUIT){
				return;
			}
		}

		if(this->_keyboardInput.wasKeyPressed(SDL_SCANCODE_F)){
			graphics.toggleFullscreen();
		}

		if(this->_keyboardInput.wasKeyPressed(SDL_SCANCODE_O)){
			this->_showFpsFlag = !this->_showFpsFlag;
		}

		if(this->_keyboardInput.wasKeyPressed(SDL_SCANCODE_I)){
			this->_vSyncFlag = !this->_vSyncFlag;
		}

		if(this->_menuStack.empty()){

			if(this->_keyboardInput.wasKeyPressed(SDL_SCANCODE_ESCAPE) || this->_keyboardInput.wasKeyPressed(SDL_SCANCODE_P) || this->_gamepadInputPlayer1.wasbuttonPressed(xbox360GamepadMaping::start)){
				this->_menuStack.emplace(new PauseMenu(graphics, this->_keyboardInput, this->_gamepadInputPlayer1));
			}

			if(this->_gamepadInputPlayer2.wasbuttonPressed(xbox360GamepadMaping::start)){
				this->_vaccumcleaner.toggleFolowingPlayer();
			}

			if(this->_keyboardInput.isKeyHeld(SDL_SCANCODE_LEFT)|| this->_gamepadInputPlayer1.isbuttonHeld(xbox360GamepadMaping::directionalLeft)){
				this->_player.moveLeft();
			}else if(this->_keyboardInput.isKeyHeld(SDL_SCANCODE_RIGHT)|| this->_gamepadInputPlayer1.isbuttonHeld(xbox360GamepadMaping::directionalRight)){
				this->_player.moveRight();
			}else if((this->_keyboardInput.isKeyHeld(SDL_SCANCODE_UP) || this->_gamepadInputPlayer1.isbuttonHeld(xbox360GamepadMaping::directionalUp)) && !(this->_keyboardInput.isKeyHeld(SDL_SCANCODE_DOWN)|| this->_gamepadInputPlayer1.isbuttonHeld(xbox360GamepadMaping::directionalDown))){
				this->_player.lookUp();
			}else if((this->_keyboardInput.isKeyHeld(SDL_SCANCODE_DOWN)|| this->_gamepadInputPlayer1.isbuttonHeld(xbox360GamepadMaping::directionalDown)) && !(this->_keyboardInput.isKeyHeld(SDL_SCANCODE_UP) || this->_gamepadInputPlayer1.isbuttonHeld(xbox360GamepadMaping::directionalUp))){
				this->_player.lookDown();
			}else{
				this->_player.stopMoving();
			}

			if(!this->_vaccumcleaner.getFolowingPlayer()){
				if(this->_gamepadInputPlayer2.isbuttonHeld(xbox360GamepadMaping::directionalLeft)){
					this->_vaccumcleaner.moveLeft();
				}else if(this->_gamepadInputPlayer2.isbuttonHeld(xbox360GamepadMaping::directionalRight)){
					this->_vaccumcleaner.moveRight();
				}else if((this->_gamepadInputPlayer2.isbuttonHeld(xbox360GamepadMaping::directionalUp)) && !(this->_keyboardInput.isKeyHeld(SDL_SCANCODE_DOWN)|| this->_gamepadInputPlayer1.isbuttonHeld(xbox360GamepadMaping::directionalDown))){
					this->_vaccumcleaner.moveUp();
				}else if((this->_gamepadInputPlayer2.isbuttonHeld(xbox360GamepadMaping::directionalDown)) && !(this->_keyboardInput.isKeyHeld(SDL_SCANCODE_UP) || this->_gamepadInputPlayer1.isbuttonHeld(xbox360GamepadMaping::directionalUp))){
					this->_vaccumcleaner.moveDown();
				}else{
					this->_vaccumcleaner.stopMoving();
				}
			}

			if(this->_keyboardInput.wasKeyPressed(SDL_SCANCODE_SPACE) || this->_gamepadInputPlayer1.wasbuttonPressed(xbox360GamepadMaping::A)){
				this->_player.jump();
			}

			if(this->_gamepadInputPlayer2.wasbuttonPressed(xbox360GamepadMaping::B)){

				if(!this->_vaccumcleaner.getFolowingPlayer()){
					this->_vaccumcleaner.bubble();
				}

			}

			if(this->_gamepadInputPlayer2.isbuttonHeld(xbox360GamepadMaping::X)){

				if(!this->_vaccumcleaner.getFolowingPlayer()){
					this->_vaccumcleaner.activateVacuum();
				}

			}

			if(this->_keyboardInput.wasKeyPressed(SDL_SCANCODE_Z) || this->_gamepadInputPlayer1.wasbuttonPressed(xbox360GamepadMaping::B)){

				if(this->_vaccumcleaner.getFolowingPlayer()){
					this->_vaccumcleaner.bubble();
				}

			}

			if(this->_keyboardInput.isKeyHeld(SDL_SCANCODE_X) || this->_gamepadInputPlayer1.isbuttonHeld(xbox360GamepadMaping::X)){

				if(this->_vaccumcleaner.getFolowingPlayer()){
					this->_vaccumcleaner.activateVacuum();
				}

			}

			if(this->_keyboardInput.wasKeyPressed(SDL_SCANCODE_C) || this->_gamepadInputPlayer1.wasbuttonPressed(xbox360GamepadMaping::Y)){

				for(std::vector<std::shared_ptr<AnimatedSprite>>::iterator it = this->_spritesToDraw.begin(); it != this->_spritesToDraw.end(); ++it) {
					float auxPosX,auxPosY, auxDesX = 0, auxDesY = 0;
					int auxWidth, auxheigth;

					this->_player.getPosSize(&auxPosX, &auxPosY, &auxWidth, &auxheigth);
					this->_player.getDes(&auxDesX, &auxDesY);

					if((*it)->checkColision(auxPosX, auxPosY, auxWidth, auxheigth, auxDesX, auxDesY)){
						(*it)->takeContextAction("Player");
						this->_player.takeContextAction((*it)->getObjectType());
					}

				}


			}

			if(this->_keyboardInput.wasKeyPressed(SDL_SCANCODE_R)){
				this->createNewPseudoRandomBlocksVector(background_blocks_constants::NUMBER_SECTORS_LINE, background_blocks_constants::NUMBER_SECTORS_COLUMN);
			}
		}else{
			this->_menuStack.top()->handleEvents();
		}

		this->update(std::min(ELAPSED_TIME_MS, MAX_FRAME_TIME));

		this->draw(graphics);

	}
}

void Game::checkColisionFullMap(){

	std::vector<ObjectQuadTree> objectQuadTreeVector;

	for(std::vector<std::shared_ptr<AnimatedSprite>>::iterator it = this->_spritesToDraw.begin(); it != this->_spritesToDraw.end(); ++it){
		objectQuadTreeVector.emplace_back(std::weak_ptr<AnimatedSprite>(*it), (*it)->getPosX(), (*it)->getPosY(), (*it)->getW(), (*it)->getH());
	}

	QuadTree auxQuadTree = QuadTree(0, 8, 5, 0, 0, this->_mapWidth, this->_mapHeight);
	/*
	 * 	inserindo player
	 */

	ObjectQuadTree playerObject = ObjectQuadTree(std::weak_ptr<AnimatedSprite>(this->_sharedPtrPlayer), this->_player.getPosX(), this->_player.getPosY(), this->_player.getW(), this->_player.getH());

	auxQuadTree.insert(&playerObject);
	/*
	 * 	inserindo vaccumcleaner
	 */

	ObjectQuadTree vaccumCleanerObject = ObjectQuadTree(std::weak_ptr<AnimatedSprite>(this->_sharedPtrVaccumCleaner), this->_vaccumcleaner.getPosX(), this->_vaccumcleaner.getPosY(), this->_vaccumcleaner.getW(), this->_vaccumcleaner.getH());

	auxQuadTree.insert(&vaccumCleanerObject);

	/*
	 * 	inserindo vaccumcone
	 */

	ObjectQuadTree vaccumConeObject = ObjectQuadTree(std::weak_ptr<AnimatedSprite>(this->_sharedPtrVaccumCone), this->_vaccumcleaner.vCone.getPosX(), this->_vaccumcleaner.vCone.getPosY(), this->_vaccumcleaner.vCone.getW(), this->_vaccumcleaner.vCone.getH());

	auxQuadTree.insert(&vaccumConeObject);


	/*
	 * 	inserindo o resto
	 */

	for(std::vector<ObjectQuadTree>::iterator it = objectQuadTreeVector.begin(); it != objectQuadTreeVector.end(); ++it){
		auxQuadTree.insert(&(*it));
	}

	auxQuadTree.runTreeCheckColisionsFull();

}

void Game::draw(Graphics &graphics){

	graphics.clear();

	for (std::vector<SectorBackground>::iterator it = this->_sectorsBackgrounds.begin() ; it != this->_sectorsBackgrounds.end(); ++it){
		 it->draw(graphics);
	}

	for (std::vector<BackgroundBlock>::iterator it = this->_backgroundBlocks.begin() ; it != this->_backgroundBlocks.end(); ++it){
		 it->draw(graphics);
	}

	for (std::vector<std::shared_ptr<AnimatedSprite>>::iterator it = this->_spritesToDraw.begin() ; it != this->_spritesToDraw.end(); ++it){
		 (*it)->draw(graphics);
	}

	/*
	 * bordas q devem aparecer embaixo do player depois tem q arrumar como vai botar isso
	 */
	for (std::vector<BackgroundBlock>::iterator it = this->_backgroundBlocks.begin() ; it != this->_backgroundBlocks.end(); ++it){
		if(it->getType() == WATER){
			it->drawBorder();
		}
	}

	this->_player.draw(graphics);

	/*
	 * bordas q devem aparecer acima do player depois tem q arrumar como vai botar isso
	 */
	for (std::vector<BackgroundBlock>::iterator it = this->_backgroundBlocks.begin() ; it != this->_backgroundBlocks.end(); ++it){
		if(it->getType() != WATER){
			it->drawBorder();
		}
	}

	this->_vaccumcleaner.draw(graphics);

	if(!this->_menuStack.empty()){
		this->_menuStack.top()->draw();
	}else{
		for (std::vector<std::shared_ptr<HUDElement>>::iterator it = this->_hudElements.begin() ; it != this->_hudElements.end(); ++it){
			 (*it)->draw(graphics);
		}
	}

	if(this->_showFpsFlag){

		SDL_Rect textRect;
		textRect.x = this->_graphicsAssociated->camera.getx() + this->_graphicsAssociated->windowWidth -32;
		textRect.y = this->_graphicsAssociated->camera.gety() ;
		textRect.w = 32;
		textRect.h = 32;

		this->_graphicsAssociated->blitSurface(this->_textCaches.getFpsTexture(std::to_string(this->_fps)), nullptr, &textRect);

		textRect.x = this->_graphicsAssociated->camera.getx() + this->_graphicsAssociated->windowWidth -64;
		textRect.y = this->_graphicsAssociated->camera.gety() + 32;
		textRect.w = 64;
		textRect.h = 32;

		if(this->_vSyncFlag){
			this->_graphicsAssociated->blitSurface(this->_textCaches.getFpsTexture("Vsync ON"), nullptr, &textRect);
		}else{
			this->_graphicsAssociated->blitSurface(this->_textCaches.getFpsTexture("Vsync OFF"), nullptr, &textRect);
		}

	}


	graphics.flip();
}

void Game::update(float elapsedtime){

	//checkar aqui colis�o do player com todos os do sprite to draw e logo em seguida tratar;

	if(this->_menuStack.empty()){
		for (std::vector<SectorBackground>::iterator it = this->_sectorsBackgrounds.begin() ; it != this->_sectorsBackgrounds.end(); ++it){
			 it->update(elapsedtime);
		}

		for (std::vector<BackgroundBlock>::iterator it = this->_backgroundBlocks.begin() ; it != this->_backgroundBlocks.end(); ++it){
			 it->update(elapsedtime);
		}

		for(unsigned int i = 0; i < this->_spritesToDraw.size(); i++){

			this->_spritesToDraw[i]->update(elapsedtime);

			if(this->_spritesToDraw[i]->getToBeDeleted()){
				std::vector<std::shared_ptr<AnimatedSprite>>::iterator it = this->_spritesToDraw.begin();
				this->_spritesToDraw.erase(it + i);
			}
		}

		this->_player.update(elapsedtime);
		this->_vaccumcleaner.update(elapsedtime);
		this->_graphicsAssociated->camera.update(elapsedtime);

		this->checkColisionFullMap();

		for (std::vector<std::shared_ptr<HUDElement>>::iterator it = this->_hudElements.begin() ; it != this->_hudElements.end(); ++it){
			 (*it)->update(elapsedtime);
		}

	}else{
		this->_menuStack.top()->update(elapsedtime);
		this->_graphicsAssociated->camera.update(elapsedtime);

		if(this->_menuStack.top()->getRequestPop()){
			this->_menuStack.pop();
			if(this->_menuToReplaceInStack){
				this->_menuStack.emplace(this->_menuToReplaceInStack);
				this->_menuToReplaceInStack = nullptr;
				this->_menuStack.top()->update(elapsedtime);
				this->_graphicsAssociated->camera.update(elapsedtime);
			}
		}
	}

}

void Game::addNewSpriteToDraw(AnimatedSprite* sprite){

	std::shared_ptr<AnimatedSprite> auxPtr(sprite);

	this->_spritesToDraw.push_back(std::move(auxPtr));

}

void Game::setupBackgroundBlocks(Graphics &graphics, int lines, int columns){

	this->_numberBlocksLine = lines;
	this->_numberBlocksColumn = columns;

	this->_backgroundBlocks.clear();

	for(int j = 0; j < columns; j++){
		for(int i = 0; i < lines; i++){
			//std::cout << i << " <- i ::" << j << " <- j " << std::endl;

			SDL_Event PingStop;
			while (SDL_PollEvent(&PingStop)) {}

			this->_backgroundBlocks.reserve(columns * lines);

			this->_backgroundBlocks.push_back(BackgroundBlock(graphics, i, j, NONE));
		}
	}

}

void Game::createNewPseudoRandomBlocksVector(int sectorsByLine, int sectorsByColumn){

	this->_graphicsAssociated->camera.folowPlayer = true;

	this->_keyboardInput.clearInputs();
	this->_gamepadInputPlayer1.clearInputs();
	this->_gamepadInputPlayer2.clearInputs();

	this->_spritesToDraw.clear();
	this->_sectorsBackgrounds.clear();
	this->_hudElements.clear();

	this->_player.setPosition(-1100, -1100);

	this->_mapWidth = ((sectorsByLine*background_blocks_constants::NUMBER_BLOCKS_LINE_SECTORS) + 2)*background_blocks_constants::BLOCK_WIDTH;
	this->_mapHeight = ((sectorsByColumn*background_blocks_constants::NUMBER_BLOCKS_COLUMN_SECTORS) + 2)*background_blocks_constants::BLOCK_HEIGTH;

	int auxX = (sectorsByLine*background_blocks_constants::NUMBER_BLOCKS_LINE_SECTORS) + 2;
	int auxY = (sectorsByColumn*background_blocks_constants::NUMBER_BLOCKS_COLUMN_SECTORS) + 2;

	this->setupBackgroundBlocks(*this->_graphicsAssociated, auxX, auxY);

	for(int j = 0; j < auxY; j++){
		for(int i = 0; i < auxX; i++){
			if((i == 0) || (j==0) || (i == auxX-1) || (j == auxY-1)){
				this->setBlockType(i,j,OUTOFBONDS);
			}
		}
	}

	/*
	 * 	inicio da cria��o do caminho obrigatorio; falta setar as flags conforme vai criando
	 */

	std::vector<Vector2> sectorWay;
	std::vector<unsigned int> sectorWayFlags;
	bool wayFound = false;
	Direction lastMove = DOWN;

	sectorWay.push_back(Vector2(rand() % sectorsByLine, 0));

	int aux;

	while(!wayFound){
		switch(lastMove){ //arrumar flags aqui
			case DOWN:

				aux = rand() % 3;

				if(aux == 2){ //left
					if(sectorWay.back().x == 0){
						sectorWay.push_back(Vector2(sectorWay.back().x,sectorWay.back().y + 1));
						sectorWayFlags.push_back(W_TOP_BOT);
					}else{
						sectorWay.push_back(Vector2(sectorWay.back().x - 1,sectorWay.back().y));
						sectorWayFlags.push_back(W_TOP_LEFT);
						lastMove = LEFT;
					}
				}else if(aux == 1){ //right
					if(sectorWay.back().x == (sectorsByLine - 1)){
						sectorWay.push_back(Vector2(sectorWay.back().x,sectorWay.back().y + 1));
						sectorWayFlags.push_back(W_TOP_BOT);
					}else{
						sectorWay.push_back(Vector2(sectorWay.back().x + 1,sectorWay.back().y));
						sectorWayFlags.push_back(W_TOP_RIGHT);
						lastMove = RIGHT;
					}
				}else{ //down
					sectorWay.push_back(Vector2(sectorWay.back().x,sectorWay.back().y + 1));
					sectorWayFlags.push_back(W_TOP_BOT);
				}

			break;
			case LEFT:

				aux = rand() % 2;

				if(aux == 1){ //left
					if(sectorWay.back().x == 0){
						sectorWay.push_back(Vector2(sectorWay.back().x,sectorWay.back().y + 1));
						sectorWayFlags.push_back(W_RIGHT_BOT);
						lastMove = DOWN;
					}else{
						sectorWay.push_back(Vector2(sectorWay.back().x - 1,sectorWay.back().y));
						sectorWayFlags.push_back(W_RIGHT_LEFT);
					}
				}else{ //down
					sectorWay.push_back(Vector2(sectorWay.back().x,sectorWay.back().y + 1));
					sectorWayFlags.push_back(W_RIGHT_BOT);
					lastMove = DOWN;
				}

			break;
			case RIGHT:

				aux = rand() % 2;

				if(aux == 1){ //right
					if(sectorWay.back().x == (sectorsByLine - 1)){
						sectorWay.push_back(Vector2(sectorWay.back().x,sectorWay.back().y + 1));
						sectorWayFlags.push_back(W_LEFT_BOT);
						lastMove = DOWN;
					}else{
						sectorWay.push_back(Vector2(sectorWay.back().x + 1,sectorWay.back().y));
						sectorWayFlags.push_back(W_LEFT_RIGHT);
					}
				}else{ //down
					sectorWay.push_back(Vector2(sectorWay.back().x,sectorWay.back().y + 1));
					sectorWayFlags.push_back(W_LEFT_BOT);
					lastMove = DOWN;
				}


			break;
			case UP:
				std::cout << "error: createNewPseudoRandomBlocksVector criou caminho impossivel" << std::endl;
				wayFound = true;
				this->requestQuit();
			break;
		}

		if(sectorWay.back().y == sectorsByColumn){
			wayFound = true;
			sectorWay.pop_back();
		}
	}

	for(int i = 0; i < (int)sectorWay.size(); i++){
		if(i == 0){
			BlockSector auxsector = this->_backgroundSectorHandler.getRandomStartSector(sectorWayFlags[i]);

			for(int ix = 0; ix < background_blocks_constants::NUMBER_BLOCKS_LINE_SECTORS; ix++){
				for(int jx = 0; jx < background_blocks_constants::NUMBER_BLOCKS_COLUMN_SECTORS; jx++){
					this->setBlockType(1 + (sectorWay[i].x*background_blocks_constants::NUMBER_BLOCKS_LINE_SECTORS) + ix , 1 + (sectorWay[i].y*background_blocks_constants::NUMBER_BLOCKS_COLUMN_SECTORS) + jx, auxsector.sectorInfo[(jx*background_blocks_constants::NUMBER_BLOCKS_LINE_SECTORS) + ix]);
				}
			}

			for(std::vector<MapObjectBlueprint>::iterator it = auxsector.objectsToBuildVector.begin(); it != auxsector.objectsToBuildVector.end(); ++it){

				this->buildMapObjectBlueprint((*it), sectorWay.at(i));

			}

			this->_sectorsBackgrounds.emplace_back(*this->_graphicsAssociated, auxsector.backgroundPath, (1 + (sectorWay[i].x*background_blocks_constants::NUMBER_BLOCKS_LINE_SECTORS)) * 64, (1 + (sectorWay[i].y*background_blocks_constants::NUMBER_BLOCKS_COLUMN_SECTORS)) * 64);

		}else if(i == (int)sectorWay.size() - 1){
			BlockSector auxsector = this->_backgroundSectorHandler.getRandomFinishSector(sectorWayFlags[i]);

			for(int ix = 0; ix < background_blocks_constants::NUMBER_BLOCKS_LINE_SECTORS; ix++){
				for(int jx = 0; jx < background_blocks_constants::NUMBER_BLOCKS_COLUMN_SECTORS; jx++){
					this->setBlockType(1 + (sectorWay[i].x*background_blocks_constants::NUMBER_BLOCKS_LINE_SECTORS) + ix , 1 + (sectorWay[i].y*background_blocks_constants::NUMBER_BLOCKS_COLUMN_SECTORS) + jx, auxsector.sectorInfo[(jx*background_blocks_constants::NUMBER_BLOCKS_LINE_SECTORS) + ix]);
				}
			}

			for(std::vector<MapObjectBlueprint>::iterator it = auxsector.objectsToBuildVector.begin(); it != auxsector.objectsToBuildVector.end(); ++it){

				this->buildMapObjectBlueprint((*it), sectorWay.at(i));

			}

			this->_sectorsBackgrounds.emplace_back(*this->_graphicsAssociated, auxsector.backgroundPath, (1 + (sectorWay[i].x*background_blocks_constants::NUMBER_BLOCKS_LINE_SECTORS)) * 64, (1 + (sectorWay[i].y*background_blocks_constants::NUMBER_BLOCKS_COLUMN_SECTORS)) * 64);

		}else{

			BlockSector auxsector = this->_backgroundSectorHandler.getRandomFillerSector(sectorWayFlags[i]);

			for(int ix = 0; ix < background_blocks_constants::NUMBER_BLOCKS_LINE_SECTORS; ix++){
				for(int jx = 0; jx < background_blocks_constants::NUMBER_BLOCKS_COLUMN_SECTORS; jx++){
					this->setBlockType(1 + (sectorWay[i].x*background_blocks_constants::NUMBER_BLOCKS_LINE_SECTORS) + ix , 1 + (sectorWay[i].y*background_blocks_constants::NUMBER_BLOCKS_COLUMN_SECTORS) + jx, auxsector.sectorInfo[(jx*background_blocks_constants::NUMBER_BLOCKS_LINE_SECTORS) + ix]);
				}
			}

			for(std::vector<MapObjectBlueprint>::iterator it = auxsector.objectsToBuildVector.begin(); it != auxsector.objectsToBuildVector.end(); ++it){

				this->buildMapObjectBlueprint((*it), sectorWay.at(i));

			}

			this->_sectorsBackgrounds.emplace_back(*this->_graphicsAssociated, auxsector.backgroundPath, (1 + (sectorWay[i].x*background_blocks_constants::NUMBER_BLOCKS_LINE_SECTORS)) * 64, (1 + (sectorWay[i].y*background_blocks_constants::NUMBER_BLOCKS_COLUMN_SECTORS)) * 64);

		}
	}

	/*
	 * 	termino da cria��o do caminho obrigatorio;
	 */

	/*
	 * 	inicio da cria��o dos setores filler
	 */
	for(int i = 0; i < sectorsByLine; i++){
		for(int j = 0; j < sectorsByColumn; j++){

			bool fillerSector = true;

			for(int k = 0; k < (int)sectorWay.size(); k++){
				if((sectorWay[k].x) == i && (sectorWay[k].y == j)){
						fillerSector = false;
				}
			}

			if(fillerSector == true){

				BlockSector auxsector = this->_backgroundSectorHandler.getRandomFillerSector();

				for(int ix = 0; ix < background_blocks_constants::NUMBER_BLOCKS_LINE_SECTORS; ix++){
					for(int jx = 0; jx < background_blocks_constants::NUMBER_BLOCKS_COLUMN_SECTORS; jx++){
						this->setBlockType(1 + (i*background_blocks_constants::NUMBER_BLOCKS_LINE_SECTORS) + ix , 1 + (j*background_blocks_constants::NUMBER_BLOCKS_COLUMN_SECTORS) + jx, auxsector.sectorInfo[(jx*background_blocks_constants::NUMBER_BLOCKS_LINE_SECTORS) + ix]);
					}
				}

				for(std::vector<MapObjectBlueprint>::iterator it = auxsector.objectsToBuildVector.begin(); it != auxsector.objectsToBuildVector.end(); ++it){

					this->buildMapObjectBlueprint((*it), Vector2(i,j));

				}

				this->_sectorsBackgrounds.emplace_back(*this->_graphicsAssociated, auxsector.backgroundPath, (1 + (i*background_blocks_constants::NUMBER_BLOCKS_LINE_SECTORS)) * 64, (1 + (j*background_blocks_constants::NUMBER_BLOCKS_COLUMN_SECTORS)) * 64);

			}
		}
	}
	/*
	 * termino da cria��o dos setores filler
	 */

	/*
	 * inicio do setup de borders
	 */

	for(int i = 0; i < this->_numberBlocksLine; i++){
		for(int j = 0; j < this->_numberBlocksColumn; j++){
			this->setupBlockBorder(i,j);
		}
	}

	/*
	 * termino do setup de borders
	 */

	/*
	 * inicio do setup do HUD
	 */

		this->_hudElements.emplace_back( new HUDPlayerHp(*this->_graphicsAssociated, &this->_player));

	/*
	 * termino do setup do HUD
	 */
}

void Game::buildMapObjectBlueprint(MapObjectBlueprint blueprint, Vector2 sectorPosition){
	switch(blueprint.type){
		case PLAYER_START_POSITION:
			this->_player.setPosition((1 + (sectorPosition.x*background_blocks_constants::NUMBER_BLOCKS_LINE_SECTORS))*background_blocks_constants::BLOCK_WIDTH + blueprint.positionOffsetOnSector.x, (1 + (sectorPosition.y*background_blocks_constants::NUMBER_BLOCKS_COLUMN_SECTORS))*background_blocks_constants::BLOCK_HEIGTH + blueprint.positionOffsetOnSector.y);
		break;

		case PORTAL_TO_NEXT_RANDOM_LEVEL:
			this->addNewSpriteToDraw( new Portal(*this->_graphicsAssociated, (1 + (sectorPosition.x*background_blocks_constants::NUMBER_BLOCKS_LINE_SECTORS))*background_blocks_constants::BLOCK_WIDTH + blueprint.positionOffsetOnSector.x, (1 + (sectorPosition.y*background_blocks_constants::NUMBER_BLOCKS_COLUMN_SECTORS))*background_blocks_constants::BLOCK_HEIGTH + blueprint.positionOffsetOnSector.y) );
		break;

		case RAT:
			this->addNewSpriteToDraw( new Rat(*this->_graphicsAssociated, (1 + (sectorPosition.x*background_blocks_constants::NUMBER_BLOCKS_LINE_SECTORS))*background_blocks_constants::BLOCK_WIDTH + blueprint.positionOffsetOnSector.x, (1 + (sectorPosition.y*background_blocks_constants::NUMBER_BLOCKS_COLUMN_SECTORS))*background_blocks_constants::BLOCK_HEIGTH + blueprint.positionOffsetOnSector.y) );
		break;
	}
}

void Game::damageBlock(int indexX, int indexY, float damage){

	if((indexX >= 0 && indexX < this->_numberBlocksLine) && (indexY >= 0 && indexY < this->_numberBlocksColumn)){
		if((this->_backgroundBlocks[indexX + (indexY*this->_numberBlocksLine)].getType() == BREAKABLE) || (this->_backgroundBlocks[indexX + (indexY*this->_numberBlocksLine)].getType() == BUBLE)){
			this->_backgroundBlocks[indexX + (indexY*this->_numberBlocksLine)].takeDamage(damage);
		}
	}
}

BlockType Game::getBlockType(int indexX, int indexY){
	if((indexX >= 0 && indexX < this->_numberBlocksLine) && (indexY >= 0 && indexY < this->_numberBlocksColumn)){
		return this->_backgroundBlocks[indexX + (indexY*this->_numberBlocksLine)].getType();
	}else{
		return NONE;
	}
}

void Game::setBlockType(int indexX, int indexY, BlockType type){

	Vector2 auxColision[4];

	float auxX, auxY;
	int auxW, auxH;

	this->_player.getPosSize(&auxX, &auxY, &auxW, &auxH);

	/*
	 * 0 = top left
	 * 1 = top right
	 * 2 = bot left
	 * 3 = bot right
	 */

	auxColision[0] = Vector2((int)(((auxX + 1)/background_blocks_constants::BLOCK_WIDTH)), (int)((auxY + 1)/background_blocks_constants::BLOCK_HEIGTH));
	auxColision[1] = Vector2((int)(((auxX + auxW - 1)/background_blocks_constants::BLOCK_WIDTH)), (int)((auxY + 1)/background_blocks_constants::BLOCK_HEIGTH));
	auxColision[2] = Vector2((int)(((auxX + 1)/background_blocks_constants::BLOCK_WIDTH)), (int)((auxY + auxH - 1)/background_blocks_constants::BLOCK_HEIGTH));
	auxColision[3] = Vector2((int)((auxX + auxW - 1)/background_blocks_constants::BLOCK_WIDTH), (int)((auxY + auxH - 1)/background_blocks_constants::BLOCK_HEIGTH));

	for(int i = 0; i < 4; i++){
		if(auxColision[i].x == indexX && auxColision[i].y == indexY){
			std::cout << "tentativa de modificar o bloco onde o player esta" << std::endl;
			return;
		}
	}

	if((indexX >= 0 && indexX < this->_numberBlocksLine) && (indexY >= 0 && indexY < this->_numberBlocksColumn)){
		this->_backgroundBlocks[indexX + (indexY*this->_numberBlocksLine)].setType(type);
	}

}

void Game::redoAdjacentsBlocksBorders(int indexX, int indexY){

	if(indexX > 0){
		this->redoBlockBorder(indexX - 1, indexY);
	}

	if(indexY > 0){
		this->redoBlockBorder(indexX, indexY - 1);
	}

	if(indexX < this->_numberBlocksLine - 1){
		this->redoBlockBorder(indexX + 1, indexY);
	}

	if(indexY < this->_numberBlocksColumn - 1){
		this->redoBlockBorder(indexX, indexY + 1);
	}

}

void Game::redoBlockBorder(int indexX, int indexY){
	this->_backgroundBlocks[indexX + (indexY*this->_numberBlocksLine)].removeBorders();
	this->setupBlockBorder(indexX, indexY);
}

void Game::setupBlockBorder(int indexX, int indexY){

	if(this->_backgroundBlocks[indexX + (indexY*this->_numberBlocksLine)].getType() == NONE){
		return;
	}

	if(indexY > 0){
		if((this->_backgroundBlocks[indexX + ((indexY - 1)*this->_numberBlocksLine)].getType() == NONE) || (this->_backgroundBlocks[indexX + ((indexY - 1)*this->_numberBlocksLine)].getType() == BUBLE)){
			this->_backgroundBlocks[indexX + (indexY*this->_numberBlocksLine)].addBorder(UP);
		}
	}

	if(indexY < this->_numberBlocksColumn - 1){
		if((this->_backgroundBlocks[indexX + ((indexY + 1)*this->_numberBlocksLine)].getType() == NONE) || (this->_backgroundBlocks[indexX + ((indexY + 1)*this->_numberBlocksLine)].getType() == BUBLE)){
			this->_backgroundBlocks[indexX + (indexY*this->_numberBlocksLine)].addBorder(DOWN);
		}
	}

	if(indexX < this->_numberBlocksLine - 1){
		if((this->_backgroundBlocks[(indexX + 1) + (indexY*this->_numberBlocksLine)].getType() == NONE) || (this->_backgroundBlocks[(indexX + 1) + (indexY*this->_numberBlocksLine)].getType() == BUBLE)){
			this->_backgroundBlocks[indexX + (indexY*this->_numberBlocksLine)].addBorder(RIGHT);
		}
	}

	if(indexX > 0){
		if((this->_backgroundBlocks[indexX -1 + (indexY *this->_numberBlocksLine)].getType() == NONE) || (this->_backgroundBlocks[indexX -1 + (indexY *this->_numberBlocksLine)].getType() == BUBLE)){
			this->_backgroundBlocks[indexX + (indexY*this->_numberBlocksLine)].addBorder(LEFT);
		}
	}

}

void Game::requestQuit(){
	this->_quitFlag = true;
}

int Game::getCurrentNumberBlocksLine(){
	return this->_numberBlocksLine;
}

int Game::getCurrentNumberBlocksColumn(){
	return this->_numberBlocksColumn;
}

void Game::setMenuToReplaceInStack(Menu* menuToReplaceInStack){
	this->_menuToReplaceInStack = menuToReplaceInStack;
}

float Game::getPlayerVaccumDps(){
	return this->_vaccumcleaner.vCone.getDps();
}

