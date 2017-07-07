#ifndef JUDGER_H
#define JUDGER_H

//------------------------------------------------------------------------
//
//  Name:   Judger.h
//
//  Desc:   A base class defining judge to the game. 
//
//  Author: Zhang qi 2017
//
//------------------------------------------------------------------------


#include "Predator.h"
#include "Prey.h"
#include "GameWorld.h"
#include "AI/RL/State.h"
#include <vector>
#include <map>

enum GameStatus{GameRunning, AllPreysInHaven, AllPreysDead, AllPredatorsDead, TimeOut};

#define m_iMaxRunTimes 10100

class Judger
{
private:
	GameWorld* m_pOwnerGame;
	GameStatus m_eCurGameStatus;

	/////////////////////////some const value for judge game/////////////////////////////////////////////////
	// once damage when prey attacks predator
	int m_iDamagePreyToP;
	// once damage when predator attacks prey
	int m_iDamagePToPrey;
	// health point gained in food zone each time
	int m_iHealthGainInFoodZ;
	//timeout value of Game for each run.
	int m_iTimeOutGame;
	//Maximial runs of Game.
	//const int m_iMaxRunTimes = 1000;

public:
	/////////////////////////////some statistic value during game running/////////////////////////////////////////////
	//number of prey agents were within a Haven zone.
	int m_iNumPreyInHaven;
	//number of prey agents were dead.
	int m_iNumPreyDead;
	//number of predator agents were dead.
	int m_iNumPredatorDead;
	//total reward
	double m_dTotalReward;

	//current episodes in game running, max up to m_iMaxRunTimes
	int m_iEpisode;
	//current ticks in an episode, max up to m_iTimeOutGame
	int m_iCurTicks;

	//average number of prey agents were within a Haven zone each run.
	double m_dAverNumPreyInHaven[m_iMaxRunTimes];
	//average number of prey agents were dead.
	double m_dAverNumPreyDead[m_iMaxRunTimes];
	//average number of predator agents were dead.
	double m_dAverNumPredatorDead[m_iMaxRunTimes];
	//average number of prey agents were alive.
	double m_dAverNumPreyAlive[m_iMaxRunTimes];
	//average number of predator agents were alive.
	double m_dAverNumPredatorAlive[m_iMaxRunTimes];

	//average reward prey agents get in each episode.
	double m_dAverReward[m_iMaxRunTimes];


public:

	Judger(int DamagePreyToP,
		   int DamagePToPrey,
		   int HealGainInFoodZ,
		   int TimeOutGame,
		   int MaxRunTimes,
		   GameWorld* pGameWorld)
	{
		m_iDamagePreyToP = DamagePreyToP;
		m_iDamagePToPrey = DamagePToPrey;
		m_iHealthGainInFoodZ = HealGainInFoodZ;

		m_iNumPreyInHaven = 0;
		m_iNumPreyDead = 0;
		m_iNumPredatorDead = 0;
		m_iEpisode = 0;
		m_iCurTicks = 0;
		m_dTotalReward = 0;

		for(int i=0;i<m_iMaxRunTimes;i++)
		{
			m_dAverNumPreyInHaven[i] = 0.0;
			m_dAverNumPreyDead[i] = 0.0;
			m_dAverNumPredatorDead[i] = 0.0;
			m_dAverNumPredatorAlive[i] = 0.0;
			m_dAverNumPreyAlive[i] = 0.0;

			m_dAverReward[i] = 0.0;
		}

		m_iTimeOutGame = TimeOutGame;
		//m_iMaxRunTimes = MaxRunTimes;
		m_pOwnerGame = pGameWorld;
		m_eCurGameStatus = GameRunning;
	}

	~Judger(void){}

	//////////////////////////////Get assessor////////////////////////////////////////////
	 int GetDamagePreyToP()const{	return m_iDamagePreyToP;}
	 int GetDamagePToPrey()const{	return m_iDamagePToPrey;}
	 int GetHealGainInFoodZ()const{ return m_iHealthGainInFoodZ;}
	 int GetMaxTimeOutEachRun()const{ return m_iTimeOutGame;}
	 int GetMaxRunTimesGame()const{ return m_iMaxRunTimes;}
	 int GetCurTicks()const {return m_iCurTicks;}

	 int GetNumPreyInHaven();
	 int GetNumPreyDead()const{return m_iNumPreyDead;}
	 int GetNumPredatorDead()const{return m_iNumPredatorDead;}
	 //double GetAverNumPreyInHaven()const{return m_dAverNumPreyInHaven;}
	 //double GetAverNumPreyDead()const{return m_dAverNumPreyDead;}
	 //double GetAverNumPredatorDead()const{return m_dAverNumPredatorDead;}

	 //////////////////////////////Set assessor////////////////////////////////////////////
	 void SetNumPreyInHaven(int NumPreyInHaven){m_iNumPreyInHaven = NumPreyInHaven;}
	 void SetNumPreyDead(int NumPreyDead){m_iNumPreyDead = NumPreyDead;}
	 void SetNumPredatorDead(int NumPredatorDead){m_iNumPredatorDead = NumPredatorDead;}

	 //////////////////////////////Judge Method////////////////////////////////////////////

	 //½»»¥±í
	 std::map<Predator*,Prey*> PtoPreyInterTable;
	 std::map<Prey*,Predator*> PreytoPInterTable;
	 std::vector<Prey*> PreyEatInterTable;

	 void AdvanceGameRuns(){m_iEpisode++;}
	 void AdvanceGameInEachRun(){m_iCurTicks++;}
	 void ResetTicksInEachRun();

	 bool UptoMaxGameRuns(){return m_iEpisode>=m_iMaxRunTimes ? true:false;}
	 GameStatus JudgeCurrentRunResult();
	 void SaveFinalStastics(char* filename);
	 void CalculateCurDamage();
	 
	 void JudgeAttackPreyToP(Prey*, Predator*) ;
	 void JudgeAttackPToPrey(Predator*, Prey*) ;

	 //////////////////////////////Some Stastic Method////////////////////////////////////////////


};

inline int Judger::GetNumPreyInHaven()
{
	//calculate the preys in haven

	m_iNumPreyInHaven = 0;

	for (unsigned int a=0; a<m_pOwnerGame->GetPreyBots().size(); ++a)
	{
		if (m_pOwnerGame->GetPreyBots()[a]->InHavenZone())
		{
			m_iNumPreyInHaven++;
		}
	}

	return m_iNumPreyInHaven;
}

//according to four conditions to end the game
inline GameStatus Judger::JudgeCurrentRunResult()
{
	//All prey agents were dead.
	if (m_pOwnerGame->GetPreyBots().size() == m_iNumPreyDead)
	{
		return AllPreysDead;
	}
	//All predator agents were dead.
	if (m_pOwnerGame->GetPredatorBots().size() == m_iNumPredatorDead)
	{
		return AllPredatorsDead;
	}
	//All prey agents were within a Haven zone.
	if (m_pOwnerGame->GetPreyBots().size()-m_iNumPreyDead == GetNumPreyInHaven())
	{
		return AllPreysInHaven;
	}
	//A timeout value was reached (set to 7500 ticks of the	update loop).
	if (m_iCurTicks >= m_iTimeOutGame)
	{
		return TimeOut;
	}

	return GameRunning;
}

inline void Judger::ResetTicksInEachRun()
{
	//m_dAverNumPreyInHaven = m_dAverNumPreyInHaven + (m_iNumPreyInHaven-m_dAverNumPreyInHaven)*1.0/m_iEpisode;
	//m_dAverNumPredatorDead = m_dAverNumPredatorDead + (m_iNumPredatorDead-m_dAverNumPredatorDead)*1.0/m_iEpisode;
	//m_dAverNumPreyDead = m_dAverNumPreyDead + (m_iNumPreyDead-m_dAverNumPreyDead)*1.0/m_iEpisode;
	//m_dAverNumPredatorAlive = m_pOwnerGame->GetPredatorBots().size() - m_dAverNumPredatorDead;
	//m_dAverNumPreyAlive = m_pOwnerGame->GetPreyBots().size() - m_dAverNumPreyDead;

	m_dAverNumPreyInHaven[m_iEpisode] = m_iNumPreyInHaven;
	m_dAverNumPreyAlive[m_iEpisode] =  m_pOwnerGame->GetPreyBots().size() - m_iNumPreyDead;
	m_dAverNumPreyDead[m_iEpisode] = m_iNumPreyDead;
	m_dAverNumPredatorDead[m_iEpisode] = m_iNumPredatorDead;
	m_dAverNumPredatorAlive[m_iEpisode] = m_pOwnerGame->GetPredatorBots().size() - m_iNumPredatorDead;

	for(int i=0;i<m_pOwnerGame->GetPreyBots().size();i++)
	{
		m_dTotalReward = m_dTotalReward + m_pOwnerGame->GetPreyBots()[i]->m_dReward;
		m_pOwnerGame->GetPreyBots()[i]->m_dReward = 0;
	}
	m_dAverReward[m_iEpisode] = m_dTotalReward/m_pOwnerGame->GetPreyBots().size();

	m_iCurTicks = 0;
	m_iNumPreyInHaven = 0;
	m_iNumPreyDead = 0;
	m_iNumPredatorDead = 0;
	m_dTotalReward = 0;
}

inline void Judger::SaveFinalStastics(char* filename)
{
	std::ofstream out;

	out.open(filename, ofstream::out);

	out.clear();

	for(int i=0;i<m_iMaxRunTimes;i++)
	{
	/*	out << "Average Number of Preys In Haven: " << m_dAverNumPreyInHaven <<endl;
		out << "Average Number of Predators Dead: " << m_dAverNumPredatorDead <<endl;
		out << "Average Number of Preys Dead: " << m_dAverNumPreyDead <<endl;
		out << "Average Number of Predators Alive: " << m_dAverNumPredatorAlive <<endl;
		out << "Average Number of Preys Alive: " << m_dAverNumPreyAlive <<endl;*/
		out << m_dAverNumPreyInHaven[i] << "    "<< m_dAverNumPreyAlive[i]<< "    "<<m_dAverNumPredatorDead[i]<<"    "<<m_dAverReward[i]<<endl;
	}

	out.close();
}

inline void Judger::CalculateCurDamage()
{
	for (std::vector<Prey*>::iterator iter=PreyEatInterTable.begin();iter!=PreyEatInterTable.end(); ++iter)
	{
		Prey* eater = *iter;
 		eater->IncreaseHealth(GetHealGainInFoodZ());
	}
	PreyEatInterTable.clear();

	for (std::map<Prey*,Predator*>::iterator iter=PreytoPInterTable.begin();iter!=PreytoPInterTable.end(); ++iter)
	{
		Prey* attacker = iter->first;
		Predator* attacked = iter->second;

 		JudgeAttackPreyToP(attacker,attacked);
	}
	PreytoPInterTable.clear();

	for (std::map<Predator*,Prey*>::iterator iter=PtoPreyInterTable.begin();iter!=PtoPreyInterTable.end(); ++iter)
	{
		Predator* attacker = iter->first;
		Prey* attacked = iter->second;

		JudgeAttackPToPrey(attacker,attacked);
	}
	PtoPreyInterTable.clear();
}


inline void Judger::JudgeAttackPreyToP(Prey* pPreyAttacker, Predator* pPredatorAttacked)
{
	//when colliding, attack is successful, reduce health point of predator
	if (Vec2DDistanceSq(pPreyAttacker->Pos(),pPredatorAttacked->Pos()) 
		< MaxOf(pPreyAttacker->Scale().x,pPreyAttacker->Scale().y)+MaxOf(pPredatorAttacked->Scale().x,pPredatorAttacked->Scale().y))
	{
		if (!pPredatorAttacked->isDead())
		{
			pPredatorAttacked->ReduceHealth(GetDamagePreyToP());

			if (pPredatorAttacked->isDead())
			{
				m_iNumPredatorDead++;
			}

			pPredatorAttacked->SetDamage(true);
		}
	}
}

inline void Judger::JudgeAttackPToPrey(Predator* pPredatorAttacker,Prey* pPreyAttacked)
{
	//when colliding, attack is successful, reduce health point of predator
	if (Vec2DDistanceSq(pPredatorAttacker->Pos(),pPreyAttacked->Pos()) 
		< MaxOf(pPredatorAttacker->Scale().x,pPredatorAttacker->Scale().y)+MaxOf(pPreyAttacked->Scale().x,pPreyAttacked->Scale().y))
	{
		if (!pPreyAttacked->isDead())
		{
			pPreyAttacked->ReduceHealth(GetDamagePToPrey());

			if (pPreyAttacked->isDead())
			{
				m_iNumPreyDead++;
			}

			pPreyAttacked->SetDamage(true);
		}
	}
}

#endif