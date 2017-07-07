#include "GameMap.h"
#include "misc/Cgdi.h"
#include "misc/WindowUtils.h"

//uncomment to write object creation/deletion to debug console
#define  LOG_CREATIONAL_STUFF
#include "debug/DebugConsole.h"


GameMap::GameMap(void): m_iSizeY(0),
						m_iSizeX(0)
{
}


GameMap::~GameMap(void)
{
	 Clear();
}

void GameMap::Clear()
{
	std::vector<Wall2D*>::iterator curWall = m_Walls.begin();
	for (curWall; curWall != m_Walls.end(); ++curWall)
	{
		delete *curWall;
	}

	m_Walls.clear();

	std::vector<BaseGameEntity*>::iterator curHaven = m_HavenZones.begin();
	for (curHaven; curHaven != m_HavenZones.end(); ++curHaven)
	{
		delete *curHaven;
	}

	m_HavenZones.clear();

	std::vector<BaseGameEntity*>::iterator curFood = m_FoodZones.begin();
	for (curFood; curFood != m_FoodZones.end(); ++curFood)
	{
		delete *curFood;
	}

	m_FoodZones.clear();
}

void GameMap::Render()
{
	//render all the walls
	std::vector<Wall2D*>::const_iterator curWall = m_Walls.begin();
	for (curWall; curWall != m_Walls.end(); ++curWall)
	{
		gdi->ThickBlackPen();
		(*curWall)->Render();
	}

	//curWall = m_HavenWalls.begin();
	//for (curWall; curWall != m_HavenWalls.end(); ++curWall)
	//{
	//	gdi->ThickBlackPen();
	//	(*curWall)->Render();
	//}

	std::vector<Vector2D>::const_iterator curSp = m_SpawnPoints.begin();
	for (curSp; curSp != m_SpawnPoints.end(); ++curSp)
	{
		gdi->GreyBrush();
		gdi->GreyPen();
		gdi->Circle(*curSp, 7);
	}

	//render all the HavenZones
	std::vector<BaseGameEntity*>::iterator curHaven = m_HavenZones.begin();
	for (curHaven; curHaven != m_HavenZones.end(); ++curHaven)
	{
		dynamic_cast<HavenZone*>(*curHaven)->Render();
	}

	//render all the FoodZones
	std::vector<BaseGameEntity*>::iterator curFood = m_FoodZones.begin();
	for (curFood; curFood != m_FoodZones.end(); ++curFood)
	{
		dynamic_cast<FoodZone*>(*curFood)->Render();
	}

}

//------------------------- LoadMap ------------------------------------
//
//  sets up the game environment from map file
//-----------------------------------------------------------------------------
bool GameMap::LoadMap(const std::string& filename)
{  
	std::ifstream in(filename.c_str());
	if (!in)
	{
		ErrorBox("Bad Map Filename");
		return false;
	}

	Clear();

	//load in the map size and adjust the client window accordingly
	in >> m_iSizeX >> m_iSizeY;


	//get the handle to the game window and resize the client area to accommodate the map
	extern char* g_szApplicationName;
	extern char* g_szWindowClassName;
	HWND hwnd = FindWindow(g_szWindowClassName, g_szApplicationName);
	const int ExtraHeightRqdToDisplayInfo = 0;
	ResizeWindow(hwnd, m_iSizeX, m_iSizeY+ExtraHeightRqdToDisplayInfo);

	//now create the environment entities
	while (!in.eof())
	{   
		//get type of next map object
		int EntityType;

		in >> EntityType;

		//create the object
		switch(EntityType)
		{
		    case type_wall:

				AddWall(in); break;

			case type_HavenZone:

				AddHavenZone(in); break;

			case type_FoodZone:

				AddFoodZone(in); break;

			case type_spawn_point:

				AddSpawnPoint(in); break;

		default:

			throw std::runtime_error("<Map::Load>: Attempting to load undefined object");

			return false;

		}//end switch
	}

	return true;
}

void GameMap::AddWall( std::ifstream& in )
{
	m_Walls.push_back(new Wall2D(in));
}

Wall2D* GameMap::AddWall(Vector2D from, Vector2D to)
{
	Wall2D* w = new Wall2D(from, to);

	m_Walls.push_back(w);

	return w;
}

void GameMap::AddSpawnPoint( std::ifstream& in )
{
	double x, y;

	in >> x >> y ;                   //dummy values are artifacts from the map editor

	m_SpawnPoints.push_back(Vector2D(x,y));
}


void GameMap::AddFoodZone( std::ifstream& in )
{
	double x,y,r;

	in >> x >> y >> r;

	int id = m_FoodZones.size();

	m_FoodZones.push_back(new FoodZone(x,y,r,id));
}

void GameMap::AddHavenZone( std::ifstream& in )
{
	double x,y,r;

	in >> x >> y >> r;

	int id = m_HavenZones.size();

	m_HavenZones.push_back(new HavenZone(x,y,r,id));

	//to make use of wall avoidance, construct haven walls
	//const int NumWallVerts = 8;
	//int border = 5;
	//Vector2D walls[8] = {Vector2D(left-border,top),
	//	Vector2D(right+border,top),
	//	Vector2D(right,top-border),
	//	Vector2D(right,bottom+border),
	//	Vector2D(right+border,bottom),
	//	Vector2D(left-border,bottom),
	//	Vector2D(left,bottom+border),
	//	Vector2D(left,top-border)};

	//for (int w=0; w<NumWallVerts/2; ++w)
	//{
	//	m_HavenWalls.push_back(new Wall2D(walls[w*2], walls[w*2+1]));
	//}
}
