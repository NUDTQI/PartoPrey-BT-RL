#pragma once
#include "Game/Region.h"
#include "Obstacle.h"


class HavenZone 
	: public Obstacle
{
private:
	int m_ID;

public:
	HavenZone(double x,
		double y,
		double r,
		int id): Obstacle(x,y,r)
	{
		m_ID = id;
	}

	~HavenZone(void){}
	inline void     Render(bool ShowID)const;
	int const ID(){return m_ID;}

};

inline void HavenZone::Render(bool ShowID = 0)const
{
	gdi->BlueBrush();
	gdi->BlackPen();
	gdi->Circle(Pos(), BRadius());

	if (ShowID)
	{ 
		gdi->TextColor(Cgdi::black);
		gdi->TextAtPos(Pos(), ttos(m_ID));
	}
}

class FoodZone
	: public Obstacle
{
private:
	int m_ID;

public:
	FoodZone(double x,
		double y,
		double r,
		int id): Obstacle(x,y,r)
	{
		m_ID = id;
	}

	~FoodZone(void){}
	inline void     Render(bool ShowID)const;
};

inline void FoodZone::Render(bool ShowID = 0)const
{
	gdi->OrangeBrush();
	gdi->PinkPen();
	gdi->Circle(Pos(), BRadius());

	if (ShowID)
	{ 
		gdi->TextColor(Cgdi::red);
		gdi->TextAtPos(Pos(), ttos(ID()));
	}
}