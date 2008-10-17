/*
*   BeHappy
*
* fenêtre d'info sur le progrès de l'update
*
*
* Version DK
*/

#include "InfoWindowDK.h"
#include <StringView.h>
#include <StatusBar.h>
#include <Button.h>

InfoWindow::InfoWindow(BRect r,BWindow *win)
	: BWindow(r,"Update Progress",B_TITLED_WINDOW,B_NOT_RESIZABLE|B_NOT_ZOOMABLE)
{
	BView *mainView = new BView(Bounds(),NULL,B_FOLLOW_ALL,B_WILL_DRAW);
	mainView->SetViewColor(216,216,216);
	AddChild(mainView);
	
	BRect sv = Bounds();
	sv.bottom = sv.top+14;
	
	line1 = new BStringView(sv,NULL,"Updating...");
	sv.top = sv.bottom;
	sv.bottom += 14;
	line2 = new BStringView(sv,NULL,"");
	sv.bottom = Bounds().bottom-30;
	sv.top = sv.bottom-30;
	bar = new BStatusBar(sv,NULL);
	
	mainView->AddChild(line1);
	mainView->AddChild(line2);
	mainView->AddChild(bar);
	
	sv.top = sv.bottom+5;
	sv.bottom = sv.top+20;
	sv.left = 0;
	sv.right = 100;
	BButton *button = new BButton(sv,NULL,"Pause",new BMessage('Paus'));
	mainView->AddChild(button);
	button->SetTarget(win);
	sv.left = 150;
	sv.right = 250;
	button = new BButton(sv,NULL,"Stop",new BMessage('Stop'));
	mainView->AddChild(button);
	button->SetTarget(win);
	
	Show();
}

InfoWindow::~InfoWindow()
{
}

void InfoWindow::MessageReceived(BMessage *message)
{
	switch(message->what)
	{
		case 'PRNT':
		{
			if (message->HasString("line1"))
				line1->SetText(message->FindString("line1"));
				
			if (message->HasString("line2"))
			{
				line2->SetText(message->FindString("line2"));
				bar->Update(1);
			}
		}
		break;
		
		case 'NPRT':
		{
			int32 n;
			if (message->FindInt32("max",&n) == B_OK)
				bar->SetMaxValue(n);
		}
		break;
		
		default:
			BWindow::MessageReceived(message);
	}
}