/* 
* BeHappy Test
*
* fichier main
*
*/

#include "HappyTest.h"
#include "TestWindow.h"
#include "BHAdd-ons.h"

#include <String.h>

int main()
{
	HappyTest myApp;
	
	myApp.Run();
}

HappyTest::HappyTest()
	: BApplication("application/x-vnd.Trap1-BeHappyTest")
{
	HTMLProject *proj = InstantiateProject(NULL);
	new TestWindow(proj,BString(projectName));
}

HappyTest::~HappyTest()
{
}

void HappyTest::MessageReceived(BMessage *message)
{
	BApplication::MessageReceived(message);
}