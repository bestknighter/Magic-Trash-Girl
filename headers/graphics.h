/*
 * graphics.h
 *
 *  Created on: Mar 29, 2018
 *      Author: TMK
 */

#ifndef SOURCE_HEADERS_GRAPHICS_H_
#define SOURCE_HEADERS_GRAPHICS_H_

#include <map>
#include <string>


struct SDL_Window;
struct SDL_Renderer;
/*
 *	Classe Graphics:
 *
 * 	Utilizada para a cria��o da janela e renderer, no projeto Trash Girl somente haver� 1 janela e somente 1 renderer,
 * 	assim esta classe ser� singleton por conveniencia.
 *
 *	\ _spriteSheets � o map que guarda os spritesheets j� carregados
 *
 */
class Graphics{

public:

	~Graphics();

	static Graphics& getInstance();

	SDL_Surface* loadImage(const std::string &filepath);

	void blitSurface(SDL_Texture* texture, SDL_Rect* sourceRectangle, SDL_Rect* destinationRectangle, double angle = 0, SDL_Point* center = nullptr, SDL_RendererFlip flip = SDL_FLIP_NONE);

	void flip();

	void clear();

	SDL_Renderer* getRenderer() const;

private:

	Graphics();

	SDL_Window* _window;
	SDL_Renderer* _renderer;

	std::map<std::string, SDL_Surface*> _spriteSheets;
};



#endif /* SOURCE_HEADERS_GRAPHICS_H_ */
