/*
*   BeHappy
*
* fenêtre d'info sur le progrès de l'update
*
*/

#include "InfoWindow.h"
#include <StringView.h>
#include <StatusBar.h>
#include <Message.h>
#include "Locale.h"
#include "BeHappy.h"

// macro utilisée pour les traductions
#define T(x) g_Locale->Translate(x)

InfoWindow::InfoWindow(BRect r)
	: BWindow(r,"Update Progress",B_MODAL_WINDOW,B_NOT_RESIZABLE)
{
	BView *mainView = new BView(Bounds(),NULL,B_FOLLOW_ALL,B_WILL_DRAW);
	mainView->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	AddChild(mainView);
	
	BRect sv = Bounds();
	sv.bottom = sv.top+14;
	
	line1 = new BStringView(sv,NULL,T("Updating..."));
	sv.top = sv.bottom;
	sv.bottom += 14;
	line2 = new BStringView(sv,NULL,"");
	sv.bottom = Bounds().bottom;
	sv.top = sv.bottom-30;
	bar = new BStatusBar(sv,NULL);
	
	mainView->AddChild(line1);
	mainView->AddChild(line2);
	mainView->AddChild(bar);
	
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