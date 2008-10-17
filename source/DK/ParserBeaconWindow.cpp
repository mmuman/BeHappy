/*
*  BeHappy Parser
*
* fenêtre pour entrer un beacon
*
*/

#include "ParserBeaconWindow.h"
#include <TextControl.h>
#include "Colors.h"

BeaconWindow::BeaconWindow(BMessage *rMessage, BMessenger *rMessenger,
		const char *str1, const char *str2)
	: BWindow(BRect(400,150,550,208),"Beacon",B_TITLED_WINDOW,B_NOT_V_RESIZABLE|B_ASYNCHRONOUS_CONTROLS)
{
	BView *mainView = new BView(Bounds(),NULL,B_FOLLOW_ALL_SIDES,B_WILL_DRAW);
	mainView->SetViewColor(BeBackgroundGrey);
	AddChild(mainView);
	
	BRect r = Bounds();
	r.left += 10;
	r.right -= 10;
	r.top += 5;
	r.bottom = r.top +24;
	string1 = new BTextControl(r,NULL,"String 1",str1,NULL,B_FOLLOW_ALL_SIDES);
	string1->SetDivider(50);
	mainView->AddChild(string1);
	
	r.top = r.bottom;
	r.bottom += 24;
	string2 = new BTextControl(r,NULL,"String 2",str2,NULL,B_FOLLOW_ALL_SIDES);
	string2->SetDivider(50);
	mainView->AddChild(string2);
	
	Show();

	returnMessenger = rMessenger;
	returnMessage = rMessage;
}

BeaconWindow::~BeaconWindow()
{
	delete returnMessenger;
	delete returnMessage;
}

bool BeaconWindow::QuitRequested()
{
	returnMessage->AddString("str1",string1->Text());
	returnMessage->AddString("str2",string2->Text());
	
	returnMessenger->SendMessage(returnMessage);
	
	return true;
}