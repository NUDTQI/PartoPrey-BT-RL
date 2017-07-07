#pragma once
#include <map>
#include <vector>

class CAction;
class CState;
class QFunction;
class StateGeneralBeSel;

using std::map;

class QLearningDataIO
{
public:
	QLearningDataIO(std::map <CAction*, QFunction*>*,std::vector<CState*>*);
	~QLearningDataIO(void);

	virtual void saveData(FILE *stream){};
	virtual void loadData(FILE *stream){};

	virtual void resetData(){};

	void SaveData(char* filename);
	bool LoadData(char* filename);

private:
	std::map <CAction*, QFunction*>* pQFunctions;
	std::vector<CState*>* pStateList;
};

