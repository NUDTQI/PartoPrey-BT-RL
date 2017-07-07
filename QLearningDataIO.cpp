#include "QLearningDataIO.h"
#include "misc/WindowUtils.h"
#include <fstream>
#include "QLearning.h"

using namespace std;

QLearningDataIO::QLearningDataIO(std::map <CLocalAction*, QFunction*>* pqf)
{
	pQFunctions = pqf;
}


QLearningDataIO::~QLearningDataIO(void)
{
}

void QLearningDataIO::SaveData(char* filename )
{
	std::ofstream out;

	out.open(filename, ofstream::out);


	for(std::map <CLocalAction*, QFunction*>::iterator it = QFunctions.begin();it!=QFunctions.end();it++)
	{
		QFunction* vf = it->second;
		vf->qf.insert(std::make_pair(pS,0));
	}

	out.close();
}

bool QLearningDataIO::LoadData(char* filename )
{
	std::ifstream in(filename);
	if (!in)
	{
		ErrorBox("Bad Data Filename");
		return false;
	}

	while (!in.eof())
	{   
		string SHealth,SNumAllyNeigh,SDisNearestFood,SDisNearestHaven,SDisNearestEnemy;

		in >> SHealth >> SNumAllyNeigh >> SDisNearestFood >> SDisNearestHaven >> SDisNearestEnemy;


	}

	in.close();

	return true;
}
