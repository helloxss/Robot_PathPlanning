#pragma once

#include <string>
#include <vector>
#include <iostream>
#include "Node.h"
#include "Grid.h"

//struct Cell {
//	Cell() {};
//	Cell(int index, Cell* parent) { 
//		this->index = index;
//		ptrParent = parent;
//	};
//	int index;
//	int iState;
//	float fScore;
//	Cell* ptrParent;
//};

struct Line {
	Line(Point start, Point end) {
		this->start = start;
		this->end = end;
	}
	Line(int x1, int y1, int x2, int y2) {
		this->start.x = x1;
		this->start.y = y1;
		this->end.x = x2;
		this->end.y = y2;
	}

	Point start;
	Point end;
};

class MapReader {
private:
	void placeLine(Point start, Point end, Grid* grid);
	void placeLine(Point start, Point end, Grid* grid, Point minOffset);
	void placeLine(Line line, Grid* grid, Point minOffset);

	
public:
	MapReader();
	void saveGrid(Grid* grid, std::string file);
	bool readIntoGrid(std::string filename, Grid* grid);

	Point m_pStartPos;
	Point m_pGoalPos;
	float m_fTH;
};