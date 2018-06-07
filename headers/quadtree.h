/*
 * quadtree.h
 *
 *  Created on: Jun 7, 2018
 *      Author: TMK
 */

#ifndef SOURCE_HEADERS_QUADTREE_H_
#define SOURCE_HEADERS_QUADTREE_H_

#include "globals.h"
#include <vector>

class QuadTree;

struct ObjectQuadTree {
	// n�o sei se index eh melhor que iterator aqui, estou usando index, mas iterators seriam melhores pois eu sei que o tamanho do vetor n�o ser� modificado
	int vectorIndex;

	float posX, posY;

	int width, height;

	QuadTree* nodeAssociated;

	ObjectQuadTree(int vectorIndex, float posX, float posY, int width, int height ):
		vectorIndex(vectorIndex), posX(posX), posY(posY), width(width), height(height), nodeAssociated(nullptr)
	{}
};

class QuadTree{
public:

	QuadTree(int layer, int maxLayers, int maxObjectsInQuadrant,float posX, float posY, int width, int height, QuadTree* fatherNode = nullptr);

	~QuadTree();

	void insert(ObjectQuadTree* objectToInsert); //nao implementado

	void clear();

private:

	void _split(); //nao implementado

	QuadTree* _fatherNode;

	std::vector<QuadTree> _nodesVector;

	//utilizando raw vectors porque os objetos s�o Local variables e tem scope e tempo de vida delimitados
	std::vector<ObjectQuadTree*> _objectVector;

	int _layer;
	int _maxLayers;
	int _maxObjectsInQuadrant;
	float _posX, _posY;
	int _w, _h;

};



#endif /* SOURCE_HEADERS_QUADTREE_H_ */
