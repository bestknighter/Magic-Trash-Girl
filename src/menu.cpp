/*
 * menu.cpp
 *
 *  Created on: May 28, 2018
 *      Author: TMK
 */


#include "menu.h"

Menu::Menu(Graphics &graphics, KeyboardInput &keyboardInput, GamepadInput &gamepadInput):
	_graphicsAssociated(&graphics),
	_keyboardInput(&keyboardInput),
	_gamepadInput(&gamepadInput),
	_requestPop(false),
	_x(0),
	_y(0),
	_w(0),
	_h(0){

}

Menu::~Menu(){

}

void Menu::update(float elapsedTime){
	this->_background.update(elapsedTime);

	this->_background.getPosSize(&this->_x, &this->_y, &this->_w, &this->_h);

	if(!this->_buttonsVector.empty()){
		for (std::vector<MenuButton>::iterator it = this->_buttonsVector.begin() ; it != this->_buttonsVector.end(); ++it){
				 it->update(elapsedTime);
			}
	}
}

void Menu::draw(){
	this->_background.draw((*this->_graphicsAssociated));

	if(!this->_buttonsVector.empty()){
		for (std::vector<MenuButton>::iterator it = this->_buttonsVector.begin() ; it != this->_buttonsVector.end(); ++it){
			it->draw(*this->_graphicsAssociated);
		}
	}

}

void Menu::addButton(){

}

bool Menu::getRequestPop(){
	return this->_requestPop;
}

void Menu::getPosSize(float* x, float* y, int* w, int* h){
	if(x != nullptr){
		*x = this->_x;
	}
	if(y != nullptr){
		*y = this->_y;
	}
	if(w != nullptr){
		*w = this->_w;
	}
	if(h != nullptr){
		*h = this->_h;
	}
}
