// teste les commandes de scripting de BeHappy

#ifndef SCRIPTTEST_H
#define SCRIPTTEST_H

#include "HappyCommander.h"
#include <Application.h>

class ScriptTest : public BApplication
{
public:
	ScriptTest();
	~ScriptTest();
	
	void ReadyToRun();

private:
	void ProcessError(int error);
	HappyCommander hc;
};
	
#endif //SCRIPTTEST_H
