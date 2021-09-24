#pragma once
#include <SFML/Graphics.hpp>
#include <iostream>
using namespace std;
#include <mutex>
#include <memory>
#include <2Darray.hpp>

class Graph {
public:
	Graph(int ys, int xs, float i_accuracy);

	void generateWorldGraph(bool** isUseable);
	int getIndexFromCoords(int y, int x, bool moveableRelevant) const;
	void disableObjectBounds(int y, int x, int width, int height);
	void moveObject(int y, int x, int oldY, int oldX, int width, int height);
	void enableObjectBounds(int y, int x, int width, int height);

	//debugging
	vector<int> deactivatedX;
	vector<int> deactivatedY;
	void debugDrawing();

	void bindNeighbour(int yIndex, int xIndex);
	void findNextUseableCoords(int* io_x, int* io_y, bool moveableRelevant) const;
	bool isDisabled(int index) const {
		return usedByMoveable[index];
	}

	inline const shared_ptr<int[]> getIndexBoundYs() const {
		return indexBoundYs;
	}

	inline const shared_ptr<int[]> getIndexBoundXs() const {
		return indexBoundXs;
	}

	inline const shared_ptr<const array2D<int>> getIndexNeighbourCosts() const {
		return neighbourCosts;
	}

	inline const shared_ptr<const array2D<int>> getNeighbourIndices() const {
		return neighbourIndices;
	}


	inline int getGraphNodeCount() {
		return graphNodeCount;
	}

	inline shared_ptr<int[]> getIndexNeighbourCount() {
		return neighbourCount;
	}

	inline shared_ptr<int[]> getHeapIndices() {
		return heapIndices;
	}

	inline void setHeapIndices(int index, int heapIndex) {
		heapIndices[index] = heapIndex;
	}

	inline shared_ptr<const bool[]> isUsedByMoveableObject() {
		return usedByMoveable;
	}

	inline void resetHeapIndices() {
		for (int nodeIndex = 0; nodeIndex < graphNodeCount; nodeIndex++) {
			heapIndices[nodeIndex] = -1;
		}
	}

	inline void setNeighbourCost(int nodeIndex, int neighbourIndex, int cost) {
		(*neighbourCosts)[nodeIndex][neighbourIndex] = cost;
	}

private:
	int findNextUseableVertex(int y, int x, bool moveableRelevant) const;

	//for getIndexFromCoords() and getting coords of indices
	int yCount;
	int xCount;
	float accuracy;

	shared_ptr<array2D<int>> rawIndices;
	shared_ptr<array2D<int>> neighbourCosts;
	shared_ptr<array2D<int>> neighbourIndices;

	int graphNodeCount;//lenght for all of these arrays
	shared_ptr<bool[]> usedByMoveable;
	shared_ptr<int[]> indexBoundXs;
	shared_ptr<int[]> indexBoundYs;
	shared_ptr<int[]> neighbourCount;
	shared_ptr<int[]> heapIndices;
};
