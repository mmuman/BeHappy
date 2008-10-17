/*
*  BeHappy Parser
*
* fenêtre pour entrer un beacon
*
*/

#include <Window.h>
class BTextControl;

class BeaconWindow : public BWindow
{
public:
	BeaconWindow(BMessage *returnMessage, BMessenger *returnMessenger,
		const char *string1, const char *string2);
	~BeaconWindow();
	bool QuitRequested();
	
private:
	BMessenger *returnMessenger;
	BMessage *returnMessage;
	
	BTextControl *string1,*string2;
};
