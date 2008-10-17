/*
* BeHappy
*
* fenêtre principale du parser
*/

#include "ParserWindow.h"
#include <Application.h>
#include "Colors.h"
#include <TextControl.h>
#include <Button.h>
#include "ColumnListView/ColumnListView.h"
#include "ColumnListView/CLVColumn.h"
#include "ColumnListView/CLVEasyItem.h"
#include <StringView.h>
#include <FilePanel.h>
#include <Path.h>
#include <Alert.h>
#include <Roster.h>
#include "ParserBeaconWindow.h"
#include "HTMLFile.h"
#include "HTMLProject.h"


ParserWindow::ParserWindow()	: 
	BWindow (BRect(100,100,500,500),"BeHappy Parser",B_DOCUMENT_WINDOW,
		B_ASYNCHRONOUS_CONTROLS)
{
	BView *mainView = new BView(Bounds(),NULL,B_FOLLOW_ALL_SIDES,B_WILL_DRAW);
	mainView->SetViewColor(BeBackgroundGrey);
	AddChild(mainView);
	
	BRect r = Bounds();
	r.right -= 100;
	r.left = 10;
	r.top += 10;
	r.bottom = r.top + 24;
	fileName = new BTextControl(r,NULL,"File to parse","",NULL,B_FOLLOW_LEFT_RIGHT|B_FOLLOW_TOP);
	fileName->SetDivider(80);
	mainView->AddChild(fileName);
	
	r.left = r.right + 10;
	r.right = Bounds().right-10;
	mainView->AddChild(new BButton(r,NULL,"Change",new BMessage('Chan'),B_FOLLOW_RIGHT|B_FOLLOW_TOP));
	
	r.left = Bounds().left + 10;
	r.right = r.left + 100;
	r.top = r.bottom+8;
	r.bottom = r.top+24;
	mainView->AddChild(new BButton(r,NULL,"View HTML",new BMessage('VHtm')));
	r.left = r.right + 10;
	r.right = r.left + 100;
	mainView->AddChild(new BButton(r,NULL,"View text",new BMessage('VTxt')));
	r.left = r.right + 10;
	r.right = r.left +100;
	mainView->AddChild(new BButton(r,NULL,"Parse!",new BMessage('Pars')));

	r.left = Bounds().left;
	r.right = Bounds().right;
	r.top = r.bottom+16;
	r.bottom = r.top+24;
	BStringView *str = new BStringView(r,NULL,"Beacons",B_FOLLOW_LEFT_RIGHT|B_FOLLOW_TOP,B_WILL_DRAW|B_FULL_UPDATE_ON_RESIZE);
	str->SetAlignment(B_ALIGN_CENTER);
	mainView->AddChild(str);
	
	r.top = r.bottom;
	r.bottom = r.top +100-14;
	float bcb = r.bottom + 14;
	r.right -= 100;
	CLVContainerView *container;
	beacons = new ColumnListView(r,&container,NULL,B_FOLLOW_LEFT_RIGHT|B_FOLLOW_TOP);
	mainView->AddChild(container);
	beacons->AddColumn(new CLVColumn("string1",100));
	beacons->AddColumn(new CLVColumn("string2",100));
	
	r.left = r.right +10+14;
	r.right = Bounds().right-10;
	r.bottom = r.top+24;
	mainView->AddChild(new BButton(r,NULL,"Add",new BMessage('Add '),B_FOLLOW_RIGHT));
	r.top = r.bottom+8;
	r.bottom = r.top+24;
	mainView->AddChild(new BButton(r,NULL,"Modify",new BMessage('Modi'),B_FOLLOW_RIGHT));
	r.top = r.bottom+8;
	r.bottom = r.top+24;
	mainView->AddChild(new BButton(r,NULL,"Delete",new BMessage('Dele'),B_FOLLOW_RIGHT));

	r.top = bcb+14;
	r.left = Bounds().left;
	r.right = Bounds().right;
	r.bottom = r.top+24;
	str = new BStringView(r,NULL,"Labels",B_FOLLOW_LEFT_RIGHT|B_FOLLOW_TOP,B_WILL_DRAW|B_FULL_UPDATE_ON_RESIZE);
	str->SetAlignment(B_ALIGN_CENTER);
	mainView->AddChild(str);
	
	r.top = r.bottom;
	r.bottom = Bounds().bottom-14;
	r.right -= 14;
	labels = new ColumnListView(r,&container,NULL,B_FOLLOW_ALL_SIDES);
	mainView->AddChild(container);
	labels->AddColumn(new CLVColumn("type",90,CLV_HEADER_TRUNCATE|CLV_TELL_ITEMS_WIDTH));
	labels->AddColumn(new CLVColumn("name",90,CLV_HEADER_TRUNCATE|CLV_TELL_ITEMS_WIDTH));
	labels->AddColumn(new CLVColumn("text",90,CLV_HEADER_TRUNCATE|CLV_TELL_ITEMS_WIDTH));
	labels->AddColumn(new CLVColumn("file",90,CLV_HEADER_TRUNCATE|CLV_TELL_ITEMS_WIDTH));

	Show();
	
	panel = new BFilePanel(B_OPEN_PANEL,new BMessenger(this),NULL,B_FILE_NODE,false);
}

ParserWindow::~ParserWindow()
{
	delete panel;
}

bool ParserWindow::QuitRequested()
{
	be_app->PostMessage(B_QUIT_REQUESTED);
	
	return true;
}

void ParserWindow::MessageReceived(BMessage *message)
{
	switch(message->what)
	{
		case 'Chan':
			panel->Show();
			break;
		
		case B_REFS_RECEIVED:
		{
			entry_ref ref;
			if (message->FindRef("refs",&ref) == B_OK)
			{
				BEntry entry(&ref);
				BPath path(&entry);
				
				Lock();
				fileName->SetText(path.Path());
				Unlock();
			}
		}
		break;
		
		case 'VHtm':
		{
			BMessage mess(B_REFS_RECEIVED);
			BEntry entry(fileName->Text());
			entry_ref ref;
			entry.GetRef(&ref);
			mess.AddRef("refs",&ref);
			
			be_roster->Launch("text/html",&mess);
		}
		break;
		
		case 'VTxt':
		{
			BMessage mess(B_REFS_RECEIVED);
			BEntry entry(fileName->Text());
			entry_ref ref;
			entry.GetRef(&ref);
			mess.AddRef("refs",&ref);
			
			be_roster->Launch("text/plain",&mess);
		}
		break;
	
		case 'Add ':
		{
			BMessage *mess = new BMessage('DoAd');
			mess->AddInt32("num",beacons->CurrentSelection());
			new BeaconWindow(mess,new BMessenger(this),"","");
		}
		break;
		
		case 'DoAd':
		{
			int32 itNum;
			const char *str1,*str2;
			if ((message->FindInt32("num",&itNum) == B_OK ) &&
				(message->FindString("str1",&str1) == B_OK ) &&
				(message->FindString("str2",&str2) == B_OK ))
			{
				CLVEasyItem *it = new CLVEasyItem;
				it->SetColumnContent(0,str1);
				it->SetColumnContent(1,str2);
				
				if (itNum < 0)
					beacons->AddItem(it);
				else
					beacons->AddItem(it,itNum);
			}
		}
		break;
		
		case 'Modi':
		{
			int itNum = beacons->CurrentSelection();
			if (itNum<0)
				return;
				
			CLVEasyItem *item = (CLVEasyItem*)beacons->ItemAt(itNum);
			BMessage *mess = new BMessage('DoMo');
			mess->AddInt32("num",itNum);
			new BeaconWindow(mess,new BMessenger(this),
				item->GetColumnContentText(0),item->GetColumnContentText(1));
		}
		break;
		
		case 'DoMo':
		{
			int32 itNum;
			const char *str1,*str2;
			if ((message->FindInt32("num",&itNum) == B_OK ) &&
				(message->FindString("str1",&str1) == B_OK ) &&
				(message->FindString("str2",&str2) == B_OK ))
			{
				CLVEasyItem *item = (CLVEasyItem*)beacons->ItemAt(itNum);
				if (item == NULL)
					return;
				
				item->SetColumnContent(0,str1);
				item->SetColumnContent(1,str2);
				
				beacons->Invalidate();
			}		
		}
		break;
		
		case 'Dele':
		{
			int itNum = beacons->CurrentSelection();
			if (itNum<0)
				return;
				
			delete beacons->RemoveItem(itNum);
		}
		break;
		
		case 'Pars':
			Parse();
		break;
		
		default:
			inherited::MessageReceived(message);	
	}
}

typedef HTMLBeacon *pHTMLBeacon;

void ParserWindow::Parse()
{
	BPath path(fileName->Text()),parent;
	path.GetParent(&parent);
	
	HTMLProject myProject;
	BDirectory root(parent.Path());
	myProject.SetHome(root);
	HTMLFile myFile(&myProject,path.Leaf());
	
	// on vide la liste
	CLVListItem *it;
	while ((it = (CLVListItem*)labels->RemoveItem((int32)0)) != NULL)
		delete it;
	
	unsigned int nBeac = beacons->CountItems();
	pHTMLBeacon *beaconsTable = new pHTMLBeacon[nBeac];
	
	for (unsigned int i=0; i<nBeac; i++)
	{
		CLVEasyItem *item = (CLVEasyItem*)beacons->ItemAt(i);
		beaconsTable[i] = new HTMLBeacon(item->GetColumnContentText(0),item->GetColumnContentText(1));

		myFile.AddBeacon(beaconsTable[i]);
	}
	
	if (myFile.Search() != B_OK)
	{
		BAlert *myAlert = new BAlert("BeHappy Parser","Error while reading file","OK",NULL,NULL,B_WIDTH_AS_USUAL,B_STOP_ALERT);
		myAlert->Go();
		return;
	}
	
	for (unsigned int i=0; i<nBeac+1; i++)
	{
		BList links(10),labls(10);
		myFile.GetLists(i,i+1,&links,&labls);

		unsigned int nL = links.CountItems();
		for (unsigned int j=0; j<nL; j++)
		{
			HTMLLabel *label = (HTMLLabel*)links.ItemAt(j);
			CLVEasyItem *it = new CLVEasyItem;
			it->SetColumnContent(0,"Link");
			it->SetColumnContent(1,label->LabelName());
			it->SetColumnContent(2,label->LabelText());
			it->SetColumnContent(3,label->FileName());
			
			labels->AddItem(it);
		}
		
		nL = labls.CountItems();
		for (unsigned int j=0; j<nL; j++)
		{
			HTMLLabel *label = (HTMLLabel*)labls.ItemAt(j);
			CLVEasyItem *it = new CLVEasyItem;
			it->SetColumnContent(0,"Label");
			it->SetColumnContent(1,label->LabelName());
			it->SetColumnContent(2,label->LabelText());
			
			labels->AddItem(it);		
		}	
		
		if ((i<nBeac) && (beaconsTable[i]->Found()))
		{
			CLVEasyItem *it = new CLVEasyItem;
			it->SetColumnContent(0,"Beacon");
			
			// on récupère la string1 du beacon
			CLVEasyItem *item = (CLVEasyItem*)beacons->ItemAt(i);
			it->SetColumnContent(1,item->GetColumnContentText(0));
			
			it->SetColumnContent(2,beaconsTable[i]->FoundString());
			
			labels->AddItem(it);
		}
	}
		
	delete beaconsTable;
}
