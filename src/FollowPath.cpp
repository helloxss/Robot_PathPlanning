#include <iostream>
#include <stdlib.h>

#include <Aria.h>

#include "FollowPath.h"

#include <cstdlib>			//rand and srand
#include <time.h>

static const double PI = 3.14159265359;

void FollowPath::simulateEncoders()
{
	m_newTs = GetTickCount();

	long long unsigned int oldt = (long long unsigned int)m_oldTs;   // Last time click as 64bit 
	long long unsigned int newt = (long long unsigned int)m_newTs;  // New time click as 64bit 
	int diff = newt - oldt; // Time since last calculation

	double d1 = ((double)diff * myRobot->getLeftVel()) / 1000.0f; // D = speed * time / 1000000.0 to get into seconds
	double d2 = ((double)diff * myRobot->getRightVel()) / 1000.0f;

	m_t1 = floor((d1 * m_tr) / (2.0f * 3.14f * m_rw)); // Rearranged odmetry equation, round as ticks would always be ints
	m_t2 = floor((d2 * m_tr) / (2.0f * 3.14f * m_rw));

	m_oldTs = m_newTs; // Set old time
}

void FollowPath::calcOdometry()
{
	//Aria readings
	m_fRX = myRobot->getX();
	m_fRY = myRobot->getY();
	m_fRTh = myRobot->getTh(); // (in degrees)

	simulateEncoders();

	double dist = 0.0f;
	double dx = 0.0f;
	double dy = 0.0f;
	double dTh = 0.0f;

	float fRadians = m_fTh * PI / 180.0f;

	dx = cos(fRadians) * (m_t1 + m_t2) * ((PI * m_rw) / m_tr);
	dy = sin(fRadians) * (m_t1 + m_t2) * ((PI * m_rw) / m_tr);

	dTh = fRadians + (2 * PI) * (m_t2 - m_t1) * (m_rw / (m_tr * m_l));
	
	
	m_fTh = dTh;
	//if (myRobot->getHeadingDoneDiff() > 0) m_fTh += dTh;
	m_fTh = m_fTh * (180 / PI);	//Convert to degrees
	//m_fTh = (int)(m_fTh) % 360;
	if (m_fTh > 180) m_fTh -= 179;
	else if (m_fTh < -180) m_fTh += 180;

	if (myRobot->getVel() > 0) {
		m_fX += dx;
		m_fY += dy;
		m_fTravelled += abs(dx);
		m_fTravelled += abs(dy);
	}
	//std::cout << dx << " " << dy << "\n";
	//std::cout << m_fX << " " << m_fY << " " << m_fTh <<  "\n";
}

void FollowPath::init()
{
	////////////	Generate Grid Map	///////////
	//MapReader mapReader;		//Create grid using map file
	//std::string sMapResLoc = "resources/maps/";
	//std::string sGridResLoc = "resources/grids/";
	//std::string sMapName = "Mine";

	//std::cout << "Generating Grid\n";
	//std::string sMapFile = (sMapResLoc.append(sMapName)).append(".map");
	//mapReader.createGrid(sMapFile, m_ptrGrid, 100);
	//

	//m_pStartPos = mapReader.m_pStartPos;

	//AStar pathFinder;
	//pathFinder.addTraversable(1, 0);
	//pathFinder.getPath(mapReader.m_pStartPos, mapReader.m_pGoalPos, m_ptrGrid, m_ptrviPath);

	//mapReader.saveGrid(m_ptrGrid, "resources/grids/" + sMapName + ".txt");
}

FollowPath::FollowPath() : ArAction("FollowPath")
{
	m_state = State::Loading;
}

ArActionDesired * FollowPath::fire(ArActionDesired d)
{
	desiredState.reset();

	m_fX = myRobot->getX() + m_ptrGrid->pMapStart.x;
	m_fY = myRobot->getY() + m_ptrGrid->pMapStart.y;
	m_fTh =  myRobot->getTh();

	//calcOdometry();

	float fOffset = 1.0f;
	float m_fRotateHeading;
	float fPositionOffset = 50.0f;
	Point pDiff;
	float fDistance = 0;
	switch (m_state)
	{
	case Idle: {
		if (!m_ptrviPath->empty()) {
			m_fDesiredPos = m_ptrGrid->vNodes.at(m_ptrviPath->front())->m_pMapCoord;
			//Calculate heading towards desired position
			float fDiff; //Angle between two points
			fDiff = atan2(m_fDesiredPos.y - m_fY, m_fDesiredPos.x - m_fX) * 180 / PI;	//Angle in degrees
			fDiff -= m_ptrGrid->fStartTh;
			m_fDesiredHeading = (int)(fDiff) % 360;
			if (m_fDesiredHeading > 180) m_fDesiredHeading -= 180;

			m_ptrviPath->erase(m_ptrviPath->begin());
			m_state = Rotating;
			m_bNodeSet = true;
		}
		else {
			std::cout << "Goal Reached\n";
		}
		break;
	}
	case FollowPath::Rotating: {
		desiredState.setVel(0);
		float fAngleDiff = m_fDesiredHeading - m_fTh;
		if (fAngleDiff > -fOffset && fAngleDiff < fOffset) {
			m_state = State::Forward;
		}
		else {
			if (fAngleDiff > 0) desiredState.setDeltaHeading(10);
			else desiredState.setDeltaHeading(-10);
		}
		break;
	}
	case FollowPath::Forward: {
		
		//Check if desired position has been reached
		pDiff = Point(m_fDesiredPos.x - m_fX, m_fDesiredPos.x - m_fY);
		fDistance = sqrt(pow(pDiff.x, 2) + pow(pDiff.y, 2));
		fDistance = sqrt(pow(m_fDesiredPos.x - m_fX, 2) + pow(m_fDesiredPos.y - m_fY,2));
		
		std::cout << fDistance << "\n";
		if (fDistance < 250) {
			m_state = State::Idle;
		}

		//Move Forward
		desiredState.setVel(200);
		break;
	}
	case Loading: {
		init();
		m_state = State::Idle;
		//myRobot->setHeading(m_ptrGrid.fStartTh);
		break;
	}
	case Test: {
		desiredState.setVel(200.0f);
		break;
	}
	default:
		break;
	}

	std::cout << "X:" << m_fX << " Y:" << m_fY << " th:" << m_fTh << "\n";
	std::cout << m_fDesiredPos.x << " " << m_fDesiredPos.y << " " << fDistance << " " << m_fDesiredHeading << "\n";


	return &desiredState;
}

void FollowPath::setPath(std::vector<int>* path, Grid * grid)
{
	m_ptrviPath = path;
	m_ptrGrid = grid;

	m_l = 425.0; // Width of a P3 DX in mm
	m_rw = 95.0; // Wheel radius in mm
	m_tr = 360.0; // Number of odemetry clicks for a full circle

	m_fX = m_ptrGrid->pMapStart.x;
	m_fY = m_ptrGrid->pMapStart.y;
	m_fTh = m_ptrGrid->fStartTh;
}

void FollowPath::setRobot(float x, float y, float th)
{
	/*m_fTh = myRobot->getTh();
	m_fX = myRobot->getX() + x;
	m_fY = myRobot->getY() + y;

	m_fTh = myRobot->getTh();
	m_fTh = (int)(m_fTh + th) % 360;
	if (m_fTh > 180) m_fTh -= 180;*/
}

void FollowPath::moveTo(float x, float y)
{
	//Find angle towards point
	float fPosX = 0.0;
	float fPosY = 0.0;
	float fThe = 0.0f;

	float fAngleDiff;

	float dot = fPosX * x + fPosY * y;
	float fMag1 = sqrt(pow(fPosX, 2) + pow(fPosY, 2));
	float fMag2 = sqrt(pow(x, 2) + pow(y, 2));

	fAngleDiff = cos(dot / (fMag1 * fMag2));
}
