#include "QLearningDataIO.h"
#include "misc/WindowUtils.h"
#include <fstream>

#include "QLearning.h"
#include "QFunction.h"
#include "CPrimitiveAction.h"
#include "State.h"
#include "../../StateBevLearning.h"

using namespace std;

QLearningDataIO::QLearningDataIO(std::map <CAction*, QFunction*>* pqf,std::vector<CState*>* psl)
{
	pQFunctions = pqf;
	pStateList = psl;
}


QLearningDataIO::~QLearningDataIO(void)
{
	pQFunctions = NULL;
	pStateList = NULL;
}

void QLearningDataIO::SaveData(char* filename )
{
	std::ofstream out;

	out.open(filename, ofstream::out);

	out.clear();

	int act;
	double qvalue;
	string blank = "    ";

	//将已有的Q表数据输出到文件中
	for(std::map<CAction*, QFunction*>::iterator it=pQFunctions->begin();it!=pQFunctions->end();it++)
	{
		CAction* va = it->first;

		act = (int)va->getAction();

		QFunction* vf = it->second;

		for (std::map<CState*, double>::iterator it2=vf->qf.begin();it2!=vf->qf.end();it2++)
		{
			CState* vs = it2->first;

			std::vector<int> rec = vs->PrintStateValues();
			qvalue = it2->second;

			out << act << blank << blank;

			for (int it3=0;it3!=rec.size();it3++)
			{
				out << rec[it3] << blank;
			}

			out << blank << blank <<qvalue <<endl;
		}
	}

	out.close();
}

bool QLearningDataIO::LoadData(char* filename )
{
	std::ifstream in(filename);
	if (!in)
	{
		ErrorBox("No QTable File exists");
		return false;
	}

	int act,iHealth,iNumAllyNeigh,iDisNearestFood,iDisNearestHaven,iDisNearestEnemy;
	double qvalue;

	while (!in.eof())
	{   
		{
			in >> act >> iHealth >> iNumAllyNeigh >> iDisNearestEnemy>>  iDisNearestHaven >>iDisNearestFood  >> qvalue;
		}

		//从已有的Q表数据文件输入到知识
		for(std::map<CAction*, QFunction*>::iterator it=pQFunctions->begin();it!=pQFunctions->end();it++)
		{
			CAction* va = it->first;
			if(act == (int)va->getAction())
			{
				QFunction* vf = it->second;

				CState* pS = new StateGeneralBevSel((HealthLevel)iHealth,(NumAllyLevel)iNumAllyNeigh,(DisLevel)iDisNearestEnemy,
					(DisLevel)iDisNearestHaven,(DisLevel)iDisNearestFood);

				if (!pStateList->empty())
				{
					for(std::vector <CState*>::iterator it = pStateList->begin();it!=pStateList->end();it++)
					{
						if ((*it)->IsSameState(pS))
						{
							vf->qf.insert(std::make_pair(*it,qvalue));
							break;
						}
					}
				}

				vf->qf.insert(std::make_pair(pS,qvalue));
				pStateList->push_back(pS);
			}
		}
	}

	in.close();

	return true;
}
