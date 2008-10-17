/*
***********************
*                     *
*       BeHappy       *
*                     *
*---------------------*
*                     *
* par Sylvain Tertois *
*                     *
***********************
*/

// Vue Netpositive

#include "NetView.h"
#include <Application.h>
#include <Message.h>
#include <Resources.h>
#include <Shelf.h>
#include <Path.h>
#include <TextControl.h>
#include <stdlib.h>
#include <Roster.h>
#include <Window.h>
#include <stdio.h>
#include <string.h>
#include <NetPositive.h>


NetView::NetView(BRect frame,const char *url)
	: BView(frame,NULL,B_FOLLOW_ALL,B_WILL_DRAW)
{
	BShelf *myShelf= new BShelf(this,false);
	
	// chargement du replicant de Netpositive
	{
		app_info meInfo;
		be_app->GetAppInfo(&meInfo);
		BFile meFile(&meInfo.ref,B_READ_ONLY);
		BResources meRessource(&meFile);
		
		size_t flatLength;
		char *flat = (char*)meRessource.LoadResource(B_RAW_TYPE,(int32)0,&flatLength);
		if (flat==NULL)
			goto error;		// oui je sais, il y en a qui aiment pas... n'empêche que je trouve ça plus élégant, na!
		
		BMessage archive;
		if (archive.Unflatten(flat) != B_OK)
			goto error;
		
		if ((archive.ReplaceRect("_frame",Bounds()) != B_OK) ||
			(archive.ReplaceString("url",url) != B_OK))
			goto error;
			
		myShelf->AddReplicant(&archive,BPoint(0,0));
	}
	
	// petits réglages sur la nouvelle vue NetPositive
	{
		BView *netpositive=ChildAt(0);
		if (netpositive==NULL)
			goto error;
		
		netpositive->SetResizingMode(B_FOLLOW_ALL_SIDES);
		
		// si on est sous la R4.5, on cherche la textview
		BView *URLView = netpositive->FindView("URLView");
		
		if ((URLView == NULL) ||
			((urlCont = (BTextControl*)(URLView->FindView("url"))) == NULL))
			// pas trouvé... on est dans la R5
			urlCont = NULL;		
	}
	return;
	
	error:
	be_app->PostMessage(B_QUIT_REQUESTED);
	return;
}

NetView::~NetView()
{
}

void NetView::NewUrl(const char *path)
{
	char *newUrl = new char[strlen(path)+10];
	sprintf(newUrl,"file://%s",path);

	NewUrlFull(newUrl);
	
	delete newUrl;
}

void NetView::NewUrlFull(const char *newUrl)
{	
	// est-on sous BeOS R5 ou R4.5?
	if (urlCont == NULL)
	{	
		// R5
	
		BView *netpositive=ChildAt(0);
		if (netpositive != NULL)
		{
			BMessage messageToNetP(B_NETPOSITIVE_OPEN_URL);
			messageToNetP.AddString("be:url",newUrl);
		
			Window()->PostMessage(&messageToNetP,netpositive);
		}
	}
	else
	{
		// R4.5
			urlCont->MakeFocus(true);
			urlCont->SetText(newUrl);

		// on envoie un message pour faire croire qu'on a appuyé sur Enter
		// c'est le seul moyen que j'ai trouvé pour faire accepter la nouvelle
		// adresse à NetPositive
		BMessage keyDown(B_KEY_DOWN);
		keyDown.AddInt64("when",(int64)system_time);
		keyDown.AddInt32("modifiers",32);
		keyDown.AddInt32("key",71);
		keyDown.AddInt8("states",0);
		keyDown.AddInt32("raw_char",10);
		keyDown.AddInt8("byte",10);
		keyDown.AddString("bytes","\n");

		Window()->PostMessage(&keyDown,NULL);	// le NULL dit à la fenêtre d'envoyer
			// le message au "prefered handler" -> la vue en focus
	}	
}
