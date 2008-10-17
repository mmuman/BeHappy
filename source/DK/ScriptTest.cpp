// teste les commandes de scripting de BeHappy

#include "ScriptTest.h"
#include <Alert.h>

#define TEST_BEBOOK

int main()
{
	ScriptTest myApp;
	myApp.Run();
}

ScriptTest::ScriptTest()
: BApplication("application/x-vnd.STertois.ScriptTest"),
#ifdef TEST_BEBOOK
  hc("Be Book","BeBook","ScriptTest.h")
#else
  hc("BeHappy Documentation","BeHappy Doc","ScriptTest.h")
#endif
{
}

ScriptTest::~ScriptTest()
{
}

void ScriptTest::ReadyToRun()
{
#ifdef TEST_BEBOOK
	(new BAlert("ScriptTest","Init","OK"))->Go();
	ProcessError(hc.InstallAddOn());
	
	(new BAlert("ScriptTest","BlistView::KeyDown()","OK"))->Go();
	ProcessError(hc.Show("By Kit","BListView","KeyDown()"));

	(new BAlert("ScriptTest","Gaga","OK"))->Go();
	ProcessError(hc.Show("Classes","BGaga","KeyDown()"));
#else
	(new BAlert("ScriptTest","Init","OK"))->Go();
	ProcessError(hc.InstallAddOn());
	
	(new BAlert("ScriptTest","Parser","OK"))->Go();
	ProcessError(hc.Show("Topics","Parser"));

	(new BAlert("ScriptTest","Dump()","OK"))->Go();
	ProcessError(hc.Show("Classes","Dump)"));
#endif
	
	PostMessage(B_QUIT_REQUESTED);
}

void ScriptTest::ProcessError(int error)
{
	const char *errText;
	switch(error)
	{
		case HappyCommander::HC_OK:
			errText = "No error";
			break;
		
		case HappyCommander::HC_ALREADY_INSTALLED:
			errText = "Add-on already installed";
			break;
		
		case HappyCommander::HC_NO_ADDON:
			errText = "Add-on not found";
			break;
			
		case HappyCommander::HC_INSTALL_ERROR:
			errText = "Install error";
			break;
		
		case HappyCommander::HC_NO_BEHAPPY:
			errText = "BeHappy not found";
			break;
			
		case HappyCommander::HC_NO_MORE_ASKING:
			errText = "No more asking";
			break;
		
		case HappyCommander::HC_REFUSED:
			errText = "refused";
			break;
		
		case HappyCommander::HC_OLD_BEHAPPY:
			errText = "Old BeHappy";
			break;
		
		case HappyCommander::HC_NO_INDEX:
			errText = "Index not found";
			break;
		
		case HappyCommander::HC_NO_TOPIC:
			errText = "Topic not found";
			break;
		
		case HappyCommander::HC_ERROR:
			errText = "Error";
			break;
	}
}