/*
*  BeHappy Test
*
* Fenêtre de test
*
*/

#include "TestWindow.h"
#include "HTMLProject.h"
#include "HTMLFile.h"
#include "HappyLink.h"
#include "InfoWindowDK.h"
#include "PathWindow.h"

TestWindow::TestWindow(HTMLProject *p, const BString &name)
	: BWindow(BRect(100,100,500,500),name.String(),B_DOCUMENT_WINDOW,B_ASYNCHRONOUS_CONTROLS)
{
	BView *mainView = new BView(Bounds(),NULL,B_FOLLOW_ALL,B_WILL_DRAW);
	mainView->SetViewColor(216,216,216);
	AddChild(mainView);
	
	list = new LinkList(BRect(0,0,386,386),NULL,B_SINGLE_SELECTION_LIST,B_FOLLOW_ALL);
	mainView->AddChild(new BScrollView(NULL,list,B_FOLLOW_ALL,0,false,true));

	string = new BStringView(BRect(0,388,400,400),NULL,"File",
		B_FOLLOW_LEFT_RIGHT|B_FOLLOW_BOTTOM,B_WILL_DRAW);
	
	mainView->AddChild(string);
	
	Show();
	proj = p;
	PostMessage('Updt');
}

TestWindow::~TestWindow()
{
}

void TestWindow::UpdateList(BStringItem *father,HappyLink *father2)
{
	HappyLink *child = father2->child;
	
	while (child != NULL)
	{
		LinkItem *listItem = new LinkItem(child->name.String(),child->link,0,false);
		list->AddUnder(listItem,father);
		UpdateList(listItem, child);
		child = child->next;
	}
}

static int32 Updater(TestWindow *win)
{
	// mise à jour du chemin
	if (!win->proj->FindHome())
	{
		BMessenger pwM(new PathWindow(win->proj));
		BMessage reply;
		
		// on attend que la fenêtre se ferme
		pwM.SendMessage('Init',&reply);
	}

	win->proj->Update(NULL);
	win->PostMessage('End_');
	
	return 0;
}

void TestWindow::MessageReceived(BMessage *message)
{
	switch(message->what)
	{
		case 'Updt':
		{
			BRect infoRect = Bounds();
			float dx = (Bounds().Width()-250)/2, dy = (Bounds().Height()-74)/2;
			infoRect.left += dx;
			infoRect.right -= dx;
			infoRect.top += dy;
			infoRect.bottom -= dy;
			infoRect = ConvertToScreen(infoRect);
			iw = new InfoWindow(infoRect,this);
			BMessenger infoMess(iw);
			proj->SetInfoMessenger(infoMess);
			
			updateThread = spawn_thread(Updater,"Updater",B_NORMAL_PRIORITY,this);
			resume_thread(updateThread);
			paused = false;
		}
		break;
		
		case 'End_':
		{
			iw->PostMessage(B_QUIT_REQUESTED);
			list->MakeEmpty();
			LinkItem *top = new LinkItem(proj->happyList->name.String(),0,0,true);
			list->AddItem(top);
			UpdateList(top,proj->happyList);
		}
		break;
		
		case 'Stop':
			kill_thread(updateThread);
			iw->PostMessage(B_QUIT_REQUESTED);
			PostMessage(B_QUIT_REQUESTED);
		break;
		
		case 'Paus':
			if (paused)
			{
				paused = false;
				resume_thread(updateThread);
			}
			else
			{
				paused = true;
				suspend_thread(updateThread);
				
				list->MakeEmpty();
				LinkItem *top = new LinkItem(proj->happyList->name.String(),0,0,true);
				list->AddItem(top);
				UpdateList(top,proj->happyList);
			}
		break;
			
		case 'NSel':
		{
			BString file(proj->GiveName(message->FindInt32("num")));
			string->SetText(file.String());
		}		
		
		default:
			BWindow::MessageReceived(message);
		break;
	}
}

bool TestWindow::QuitRequested()
{
	be_app->PostMessage(B_QUIT_REQUESTED);
	return true;
}

LinkItem::LinkItem(const char *label,unsigned int l, uint32 level, bool expanded)
	: BStringItem(label,level,expanded)
{
	link = l;
}

LinkList::LinkList(BRect frame,const char *name,list_view_type t, uint32 re, uint32 fl)
	: BOutlineListView(frame,name,t,re,fl)
{
}

void LinkList::SelectionChanged()
{
	LinkItem *item = (LinkItem*)ItemAt(CurrentSelection());
	
	if (item != NULL)
	{
		BMessage newSel('NSel');
		newSel.AddInt32("num",item->link);
		
		Window()->PostMessage(&newSel);
	}
}