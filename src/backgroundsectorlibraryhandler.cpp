/*
 * backgroundsectorhandler.cpp
 *
 *  Created on: May 24, 2018
 *      Author: TMK
 */

#include <backgroundsectorlibraryhandler.h>

BackgroundSectorLibraryHandler::BackgroundSectorLibraryHandler(){

	std::vector<BlockType> aux;
	unsigned int auxFlags;

	for(int i = 0; i < 8; i++){
		for(int j = 0; j < 8; j ++){
			if(i == 0 || i == 7 || j == 0 || j == 7){
				aux.push_back(BREAKABLE);
			}else{
				aux.push_back(NONE);
			}
		}
	}

	auxFlags = (W_TOP_BOT | W_TOP_LEFT | W_TOP_RIGHT | W_BOT_LEFT | W_BOT_RIGHT | W_LEFT_RIGHT);

	this->_fillerSectorLibrary.push_back(BlockSector(aux, auxFlags));

	aux.clear();

	for(int i = 0; i < 8; i++){
		for(int j = 0; j < 8; j ++){
			if(i == 0 || i == 7 || j == 0 || j == 7){
				aux.push_back(NONE);
			}else{
				aux.push_back(BREAKABLE);
			}
		}
	}

	auxFlags = (W_TOP_BOT | W_TOP_LEFT | W_TOP_RIGHT | W_BOT_LEFT | W_BOT_RIGHT | W_LEFT_RIGHT);

	this->_fillerSectorLibrary.push_back(BlockSector(aux, auxFlags));

	aux.clear();

	for(int i = 0; i < 8; i++){
		for(int j = 0; j < 8; j ++){
			if(i == 0 || i == 7 || j == 0 || j == 7){
				aux.push_back(BREAKABLE);
			}else{
				aux.push_back(UNBREAKABLE);
			}
		}
	}

	auxFlags = (W_TOP_BOT | W_TOP_LEFT | W_TOP_RIGHT | W_BOT_LEFT | W_BOT_RIGHT | W_LEFT_RIGHT);

	this->_fillerSectorLibrary.push_back(BlockSector(aux, auxFlags));

	aux.clear();

	for(int i = 0; i < 8; i++){
		for(int j = 0; j < 8; j ++){
			aux.push_back(BREAKABLE);
		}
	}

	auxFlags = (W_TOP_BOT | W_TOP_LEFT | W_TOP_RIGHT | W_BOT_LEFT | W_BOT_RIGHT | W_LEFT_RIGHT);

	this->_startSectorLibrary.push_back(BlockSector(aux, auxFlags));

	aux.clear();

	for(int i = 0; i < 8; i++){
		for(int j = 0; j < 8; j ++){
			aux.push_back(UNBREAKABLE);
		}
	}

	auxFlags = (W_TOP_BOT | W_TOP_LEFT | W_TOP_RIGHT | W_BOT_LEFT | W_BOT_RIGHT | W_LEFT_RIGHT);

	this->_finishSectorLibrary.push_back(BlockSector(aux, auxFlags));

}

std::vector<BlockType> BackgroundSectorLibraryHandler::getRandomFillerSector(unsigned int flags){

	int randomIndex = rand() % this->_fillerSectorLibrary.size();

	for(int i = randomIndex ; i < (int)this->_fillerSectorLibrary.size() ; i ++){
		if((this->_fillerSectorLibrary[i].sectorFlags & flags) == flags){
			return this->_fillerSectorLibrary[i].sectorInfo;
		}
	}
	for(int i = randomIndex-1 ; i >= 0 ; i --){
		if((this->_fillerSectorLibrary[i].sectorFlags & flags) == flags){
			return this->_fillerSectorLibrary[i].sectorInfo;
		}
	}

	std::cout << "error nenhum setor com as configuraçoes encontradas, colocando setor nulo no local" << std::endl;

	std::vector<BlockType> aux;

	for(int i = 0; i < 8; i++){
		for(int j = 0; j < 8; j ++){
			aux.push_back(NONE);
		}
	}

	return aux;
}

std::vector<BlockType> BackgroundSectorLibraryHandler::getRandomStarteSector(unsigned int flags){

	int randomIndex = rand() % this->_startSectorLibrary.size();

	for(int i = randomIndex ; i < (int)this->_startSectorLibrary.size() ; i ++){
		if((this->_startSectorLibrary[i].sectorFlags & flags) == flags){
			return this->_startSectorLibrary[i].sectorInfo;
		}
	}
	for(int i = randomIndex-1 ; i >= 0 ; i --){
		if((this->_startSectorLibrary[i].sectorFlags & flags) == flags){
			return this->_startSectorLibrary[i].sectorInfo;
		}
	}

	std::cout << "error nenhum setor com as configuraçoes encontradas, colocando setor nulo no local" << std::endl;

	std::vector<BlockType> aux;

	for(int i = 0; i < 8; i++){
		for(int j = 0; j < 8; j ++){
			aux.push_back(NONE);
		}
	}

	return aux;
}

std::vector<BlockType> BackgroundSectorLibraryHandler::getRandomFinishSector(unsigned int flags){

	int randomIndex = rand() % this->_finishSectorLibrary.size();

	for(int i = randomIndex ; i < (int)this->_finishSectorLibrary.size() ; i ++){
		if((this->_finishSectorLibrary[i].sectorFlags & flags) == flags){
			return this->_finishSectorLibrary[i].sectorInfo;
		}
	}
	for(int i = randomIndex-1 ; i >= 0 ; i --){
		if((this->_finishSectorLibrary[i].sectorFlags & flags) == flags){
			return this->_finishSectorLibrary[i].sectorInfo;
		}
	}

	std::cout << "error nenhum setor com as configuraçoes encontradas, colocando setor nulo no local" << std::endl;

	std::vector<BlockType> aux;

	for(int i = 0; i < 8; i++){
		for(int j = 0; j < 8; j ++){
			aux.push_back(NONE);
		}
	}

	return aux;
}

/*
 * inicio da implementação dos operadores de SectorFlags
 *
 * vide C++ standart 17.5.2.1.3 Bitmask types
 */

constexpr SectorFlags operator&(SectorFlags X, SectorFlags Y) {
    return static_cast<SectorFlags>(
        static_cast<unsigned int>(X) & static_cast<unsigned int>(Y));
}

constexpr SectorFlags operator|(SectorFlags X, SectorFlags Y) {
    return static_cast<SectorFlags>(
        static_cast<unsigned int>(X) | static_cast<unsigned int>(Y));
}

constexpr SectorFlags operator^(SectorFlags X, SectorFlags Y) {
    return static_cast<SectorFlags>(
        static_cast<unsigned int>(X) ^ static_cast<unsigned int>(Y));
}

constexpr SectorFlags operator~(SectorFlags X) {
    return static_cast<SectorFlags>(~(static_cast<unsigned int>(X)));
}

SectorFlags& operator&=(SectorFlags& X, SectorFlags Y) {
    X = X & Y; return X;
}

SectorFlags& operator|=(SectorFlags& X, SectorFlags Y) {
    X = X | Y; return X;
}

SectorFlags& operator^=(SectorFlags& X, SectorFlags Y) {
    X = X ^ Y; return X;
}
/*
 * termino da implementação dos operadores de SectorFlags
 *
 */
