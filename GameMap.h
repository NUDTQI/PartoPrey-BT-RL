#pragma once
#ifndef GAMEMAP_H
#define GAMEMAP_H

#include <vector>
#include <string>
#include <list>
#include "2d/Wall2D.h"
#include "Agent.h"
#include "EntityFunctionTemplates.h"
#include "Zone.h"

class BaseGameEntity;
class Predator;
class Prey;

enum 
{
	type_Agent,
	type_wall,
	type_HavenZone,
	type_FoodZone,
	type_spawn_point
};


class GameMap
{

private:
	//the walls that comprise the current map's architecture. 
	std::vector<Wall2D*>                m_Walls;

	//special¡ª¡ªthe Haven walls that comprise the current map's havens, to make use of avoidance. 
	//std::vector<Wall2D*>                m_HavenWalls;

	//this holds a number of spawn positions. When a bot is instantiated
	//it will appear at a randomly selected point chosen from this vector
	std::vector<Vector2D>              m_SpawnPoints;

	//any obstacles
	std::vector<BaseGameEntity*>  m_Obstacles;

	//two special game regions
	std::vector<BaseGameEntity*>   m_FoodZones;
	std::vector<BaseGameEntity*>   m_HavenZones; 

	// length and width of map
	int m_iSizeX;
	int m_iSizeY;

	//stream constructors for loading from a file
	void AddWall(std::ifstream& in);
	void AddSpawnPoint(std::ifstream& in);
	void AddFoodZone(std::ifstream& in);
	void AddHavenZone(std::ifstream& in);

	//adds a wall and returns a pointer to that wall. (this method can be
	//used by objects such as doors to add walls to the environment)
	Wall2D* AddWall(Vector2D from, Vector2D to);

	void Clear();


public:
	GameMap(void);
	~GameMap(void);

	void Render();

	//loads an environment from a file
	bool LoadMap(const std::string& FileName); 


	void  TagHavensWithinViewRange(BaseGameEntity* pAgent, double range)
	{
		TagNeighbors(pAgent, m_HavenZones, range);
	}

	void  TagObstaclesWithinViewRange(BaseGameEntity* pAgent, double range)
	{
		TagNeighbors(pAgent, m_Obstacles, range);
	}

	const std::vector<Wall2D*>&           GetWalls()const{return m_Walls;}
	const std::vector<Vector2D>&		  GetSpawnPoints()const{return m_SpawnPoints;}
	const std::vector<BaseGameEntity*>&         GetFoodZones()const{return m_FoodZones;}
	const std::vector<BaseGameEntity*>&   GetHavenZones()const{return m_HavenZones;}                         
	const std::vector<BaseGameEntity*>&   GetObstacles()const{return m_Obstacles;}
	//const std::vector<Wall2D*>&           GetHavenWalls()const{return m_HavenWalls;}  //specially
	int									  GetSizeX()const{return m_iSizeX;}
	int                                   GetSizeY()const{return m_iSizeY;}
};

#endif