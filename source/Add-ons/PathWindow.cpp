/*
* BeHappy
*
*
* Fenêtre pour modifier le chemin de base de la doc HTML
*
*/

#include "PathWindow.h"
#include "HTMLProject.h"
#include <TextControl.h>
#include <StringView.h>
#include <Button.h>
#include "Locale.h"
#include "BeHappy.h"

// macro utilisée pour les traductions
#ifndef DK_LIB
#define T(x) g_Locale->Translate(x)
#else
#define T(x) x
#endif

PathWindow::PathWindow(HTMLProject *p)
	: BWindow(BRect(150,150,550,214),"BeHappy : Add-on path",B_MODAL_WINDOW,
		B_NOT_CLOSABLE|B_NOT_ZOOMABLE|B_NOT_V_RESIZABLE| B_ASYNCHRONOUS_CONTROLS)
{
	proj = p;

	BView *mainView = new BView(Bounds(),NULL,B_FOLLOW_ALL_SIDES,B_WILL_DRAW);
	mainView->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	AddChild(mainView);
	
	// on déplace la fenêtre au niveau de la souris
	{
		BPoint mpt;
		uint32 buttons;
		mainView->GetMouse(&mpt,&buttons);
		MoveBy(mpt.x,mpt.y);
	}
	
	BRect r = Bounds();
	r.bottom = r.top+24;

	BStringView *str = new BStringView(r,NULL,T("Please give the HTML files path"),B_FOLLOW_LEFT_RIGHT|B_FOLLOW_TOP);
	str->SetAlignment(B_ALIGN_CENTER);
	mainView->AddChild(str);
	
	r.top = r.bottom+8;
	r.bottom = Bounds().bottom;
	
	BPath *thepath = proj->GetPath("");
	r.right -= 124;
	path = new BTextControl(r,NULL,NULL,thepath->Path(),NULL,B_FOLLOW_LEFT_RIGHT|B_FOLLOW_TOP);
	mainView->AddChild(path);
	
	r.left = r.right+8;
	r.right = r.left+50;
	r.bottom -= 8;
	mainView->AddChild(new BButton(r,NULL,"Browse",new BMessage('Brow'),B_FOLLOW_RIGHT|B_FOLLOW_TOP));
	
	r.left = r.right+8;
	r.right = r.left+50;
	BButton *button =  new BButton(r,NULL,"OK",new BMessage('OK  '),B_FOLLOW_RIGHT|B_FOLLOW_TOP);
	button->MakeDefault(true);
	mainView->AddChild(button);
	
	Show();
	
	BEntry entry(thepath->Path());
	entry_ref ref;
	entry.GetRef(&ref);
	panel = new BFilePanel(B_OPEN_PANEL,new BMessenger(this),&ref,B_DIRECTORY_NODE,false,NULL,NULL,true);
}

void PathWindow::MessageReceived(BMessage *message)
{
	switch(message->what)
	{
		// ce message est envoyé au moment de la création de la fenêtre
		// l'emetteur attend une réponse synchrone. Donc il est bloqué tant
		// qu'on ne répond pas à ce message
		case 'Init':
			replyToThis = DetachCurrentMessage();
			break;
		
		case 'Brow':
			panel->Show();
			break;
		
		case B_REFS_RECEIVED:
		{
			entry_ref ref;
			if (message->FindRef("refs",&ref) == B_OK)
			{
				BEntry entry(&ref);
				BPath thePath(&entry);
				
				path->SetText(thePath.Path());
			}
		}
		break;
		
		case 'OK  ':
		{
			BDirectory dir(path->Text());
			
			proj->SetHome(dir);
			
			replyToThis->SendReply('PWok');
			
			PostMessage(B_QUIT_REQUESTED);
		}
		break;
		
		default:
			BWindow::MessageReceived(message);
			break;
	}
}