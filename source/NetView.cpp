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

//#define TRACE(a) printf a
#define TRACE(a) do {} while (0)

NetView::NetView(BRect frame,const char *url)
	: BView(frame,NULL,B_FOLLOW_ALL,B_WILL_DRAW)
{
	TRACE(("NetView::%s(%s)\n", __FUNCTION__, url));
	fShelf = new BShelf(this,false);

	status_t err;
	// try NetSurf first
	err = LoadReplicant(url, 1);
	TRACE(("NetView::%s: %s\n", __FUNCTION__, strerror(err)));
	// fall back to NetPositive
	if (err < B_OK)
		err = LoadReplicant(url, 0);
	TRACE(("NetView::%s: %s\n", __FUNCTION__, strerror(err)));
	if (err < B_OK)
		goto error;		// oui je sais, il y en a qui aiment pas... n'empêche que je trouve ça plus élégant, na!

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

status_t NetView::LoadReplicant(const char *url, int32 resource)
{
	TRACE(("NetView::%s(%s, %ld)\n", __FUNCTION__, url, resource));
	// chargement du replicant de Netpositive ou NetSurf
	app_info meInfo;
	be_app->GetAppInfo(&meInfo);
	BFile meFile(&meInfo.ref,B_READ_ONLY);
	BResources meRessource(&meFile);
	status_t err;
	
	size_t flatLength;
	char *flat = (char*)meRessource.LoadResource(B_RAW_TYPE,resource,&flatLength);
	if (flat==NULL)
		return B_ERROR;
	
	BMessage archive;
	if (archive.Unflatten(flat) != B_OK)
		return B_ERROR;
	
	if ((archive.ReplaceRect("_frame",Bounds()) != B_OK) ||
		(archive.ReplaceString("url",url) != B_OK))
		return B_ERROR;

	err = fShelf->AddReplicant(&archive,BPoint(0,0));
	if (err < B_OK)
		return err;

	BView *netpositive=ChildAt(0);
	if (netpositive==NULL)
		return ENOENT;
	return B_OK;
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
