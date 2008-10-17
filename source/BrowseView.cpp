/*
*  BeHappy
*
* View avec la liste des trucs à afficher
*
*/

#include "BrowseView.h"
#include "HappyLink.h"
#include "MainWindow.h"
#include "Mover.h"
#include "Tasks.h"
#include "Eraser.h"
#include "BeHappy.h"
#include "Locale.h"

// macro utilisée pour les traductions
#define T(x) g_Locale->Translate(x)

#include <Application.h>
#include <stdio.h>

static const char clockMouse[] = {
	16,1,7,7,
	0x00,0x00,0x07,0xc0,0x08,0x20,0x10,0x10,
	0x21,0x08,0x41,0x04,0x41,0x04,0x43,0xf4,
	0x41,0x04,0x40,0x04,0x20,0x08,0x10,0x10,
	0x08,0x20,0x07,0xc0,0x00,0x00,0x00,0x00,
	0x00,0x00,0x07,0xc0,0x0f,0xe0,0x1f,0xf0,
	0x3f,0xf8,0x7f,0xfc,0x7f,0xfc,0x7f,0xfc,
	0x7f,0xfc,0x7f,0xfc,0x3f,0xf8,0x1f,0xf0,
	0x0f,0xe0,0x07,0xc0,0x00,0x00,0x00,0x00
	};

BrowseView::BrowseView(BRect r,Tasks *t)
	: BView(r,NULL,B_FOLLOW_ALL,B_WILL_DRAW)
{
	tasks = t;
	
	SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	
	mainList = NULL;
	linksList = NULL;
	horizontalMover = NULL;
	nextButton = NULL;
	searchTextView = NULL;
	containsView = NULL;
	infoText = NULL;
	m_DontUpdateURL = false;
}

BrowseView::~BrowseView()
{
}

void BrowseView::ResizeList(float coord)
{
	if (horizontalMover == NULL)
		return;
		
	BPoint p = ConvertFromScreen(BPoint(0,coord));
	mainScroll->ResizeTo(mainScroll->Frame().Width(),p.y-mainScroll->Frame().top-2);
	linksScroll->ResizeTo(linksScroll->Frame().Width(),Frame().Height()-p.y-horizontalMover->Frame().Height()-bottomSpace);
	linksScroll->MoveTo(linksScroll->Frame().left,p.y+2+horizontalMover->Frame().Height());
	horizontalMover->MoveTo(horizontalMover->Frame().left,p.y);
}

float BrowseView::GetMoverPos()
{
	if (horizontalMover == NULL)
		return -1;
	else
		return(horizontalMover->Frame().top);
}

// macro pour supprimer une vue
#define DESTROYVIEW(x)	\
	if (x != NULL)		\
	{					\
		x->RemoveSelf();\
		delete x;		\
		x = NULL;		\
	}	
		
void BrowseView::Clear()
{
	tasks->GiveUp();
	
	LockLooper();
	if (mainList != NULL)
	{
		RemoveChild(mainScroll);

		eraser->KillList(mainList,mainScroll);
		mainList = NULL;
	}
	
	if (linksList != NULL)
	{
		RemoveChild(linksScroll);

		eraser->KillList(linksList,linksScroll);
		linksList = NULL;
	}

	
	DESTROYVIEW(horizontalMover)
	DESTROYVIEW(nextButton)
	DESTROYVIEW(searchTextView)
	DESTROYVIEW(containsView)
	DESTROYVIEW(infoText)
		
	UnlockLooper();
}

void BrowseView::NewList(HappyLink *t, unsigned int mode)
{
	tasks->GiveUp();
	LockLooper();
	
	top = t;
	// on récupère le numéro du lien actuellement sélectionné
	int n = GetLinkNum();
	
	// on vide la vue actuelle
	Clear();
	
	// on la reconstruit	
	bottomSpace = 8;
	if (mode & BW_NEXTBUTTON)
		bottomSpace += 24;
	
	if (mode & BW_SEARCHTXTV)
	{
		bottomSpace += 24;
		if (mode & BW_CONTAINS)
			bottomSpace += 24;
	}
	
	if (mode & BW_INFOTEXT)
		bottomSpace += 14;

	BRect r=Bounds();
	r.top+=8;
	r.bottom-=bottomSpace;
	r.left+=8;
	r.right-=22;
		
	BRect lr(r);
	
	if (mode & BW_SECONDLIST)
	{
		lr.bottom = (r.top+r.bottom)/2-6;
		mainList = new LinkList2(lr,NULL,B_SINGLE_SELECTION_LIST,this,B_FOLLOW_ALL);

		BRect mr;
		mr.top = lr.bottom+4;
		mr.bottom = mr.top+4;
		mr.left = Bounds().left;
		mr.right = Bounds().right;
		horizontalMover = new Mover(mr,B_FOLLOW_LEFT_RIGHT|B_FOLLOW_TOP,false);
		AddChild(horizontalMover);
	
		mainScroll = new BScrollView(NULL,mainList,B_FOLLOW_LEFT_RIGHT|B_FOLLOW_TOP,0,false,true);
		AddChild(mainScroll);

		lr.top = lr.bottom+12;
		lr.bottom = r.bottom;
	}
	linksList = new LinkList(lr,NULL,this,B_SINGLE_SELECTION_LIST,B_FOLLOW_ALL);
	
	linksScroll = new BScrollView(NULL,linksList,B_FOLLOW_LEFT_RIGHT|B_FOLLOW_TOP_BOTTOM,0,false,true);
	AddChild(linksScroll);

	if (mode & BW_NEXTBUTTON)
	{
		r.top = r.bottom+4;
		r.bottom = r.top+16;
		
		nextButton = new BButton(r,NULL,T("Next"),new BMessage('NEXT'),B_FOLLOW_BOTTOM|B_FOLLOW_LEFT_RIGHT);
		AddChild(nextButton);
		
		r.bottom += 4;
	}
	
	if (mode & BW_SEARCHTXTV)
	{
		if (mode & BW_CONTAINS)
		{
			r.top = r.bottom+4;
			r.bottom = r.top+16;
		
			containsView = new BCheckBox(r,NULL,T("Containing:"),new BMessage('IDCN'),B_FOLLOW_LEFT_RIGHT|B_FOLLOW_BOTTOM);
			AddChild(containsView);
		
			r.bottom += 4;
		}

		r.top = r.bottom+4;
		r.bottom = r.top+16;
		
		searchTextView = new BTextControl(r,NULL,"","",new BMessage('IDXV'),B_FOLLOW_LEFT_RIGHT|B_FOLLOW_BOTTOM);
		searchTextView->SetDivider(-2);
		searchTextView->SetModificationMessage(new BMessage('IDXC'));
		AddChild(searchTextView);
		
		r.bottom += 4;
	}
	
	if (mode & BW_INFOTEXT)
	{
		r.top = r.bottom+2;
		r.bottom = r.top+12;
		
		infoText = new BStringView(r,NULL,"",B_FOLLOW_LEFT_RIGHT|B_FOLLOW_BOTTOM);
		AddChild(infoText);
	}
	
	containsSearch = false;
	
	BusyOn();

	if (mainList != NULL)
	{
		if (top != NULL)
		{
			RemoveChild(mainScroll);
			UpdateList(mainList,NULL,top->child);
			AddChild(mainScroll);
		}
	}
	else
	{
		if (top != NULL)
		{
			RemoveChild(linksScroll);
			UpdateList(linksList,NULL,top->child,false);
			AddChild(linksScroll);
		}
	}

	// si un lien était sélectionné avant le changement de liste, on tente de le retrouver
	if (n>=0)
		SetLinkNum(n,top);

	BusyOff();		
	UnlockLooper();
}

bool BrowseView::NewLinksList(HappyLink *top)
{
	tasks->GiveUp();
	
	linksList->MakeEmpty();
	UpdateList(linksList,NULL,top->child,true);
	
	return(!linksList->IsEmpty());
}

void BrowseView::UpdateList(LinkList *list,LinkItem *father,HappyLink *father2,bool onlyKids)
{
	HappyLink *link = father2;
	
	while (link != NULL)
	{
		if (!onlyKids || link->child == NULL)
		{
			LinkItem *listItem = new LinkItem(link->name.String(),link->link,0,false);
			listItem->info = link->info.String();
		
			// maintenant on peut mettre l'objet dans la liste
			if (father == NULL)
				list->AddItem(listItem,0);
			else	
				list->AddUnder(listItem,father);
		
			UpdateList(list,listItem,link->child,onlyKids);
		}
		link = link->next;
	}
}

void BrowseView::UpdateList(LinkList2 *list,LinkItem2 *father,HappyLink *father2)
{
	HappyLink *link = father2;
	
	while (link != NULL)
	{
		if (link->child != NULL)
		{
			LinkItem2 *listItem = new LinkItem2(link->name.String(),link,0,false);
			if (father == NULL)
				list->AddItem(listItem,0);
			else
				list->AddUnder(listItem,father);

			UpdateList(list,listItem,link->child);
		}
		
		link = link->next;
	}
}

void BrowseView::NextClicked()
{
	tasks->GiveUp();
	
	if (mainList != NULL)
	{
		const char *lk = NULL;
		if (linksList != NULL)
		{
			int n = linksList->CurrentSelection();
			if (n>=0)
				lk = ((BStringItem*)linksList->ItemAt(n))->Text();
		}
		
	
		int sel = mainList->CurrentSelection();
		if (sel >= 0)
		{
			BListItem *item = mainList->ItemAt(sel);
			NextInList(mainList,((BStringItem*)item)->Text(),item);
		}
		
		if (lk != NULL)
			NextInList(linksList,lk);
	}
	
}

void BrowseView::ContainsClicked()
{
	unsigned int oldSelected = GetLinkNum();
	
	containsSearch = containsView->Value();
	
	if (!containsSearch)
	{
		linksList->MakeEmpty();

		RemoveChild(linksScroll);
		UpdateList(linksList,NULL,top->child,false);
		AddChild(linksScroll);

		FindInIndex();
	}
	else
	{
		FindInIndex();
	}
	
	if (oldSelected >=0)
		SetLinkNum(oldSelected,top);
}

int BrowseView::GetLinkNum()
{
	// on cherche l'item sélectionné, de préférence celui dans linksList
	if (linksList != NULL)
	{
		int n = linksList->CurrentSelection();
		if (n>=0)
			return ((LinkItem*)linksList->ItemAt(n))->link;
	}
	
	// si on a pas trouvé dans la linksList, on cherche dans la mainList
	if (mainList != NULL)
	{
		int n = mainList->CurrentSelection();
		if (n>0)
			return((LinkItem2*)mainList->ItemAt(n))->link->link;
	}
	
	// on a rien trouvé
	return -1;
}

void BrowseView::SetLinkNum(int n,HappyLink *list)
{
	HappyLink *father,*sun;
	m_DontUpdateURL = true;
	if (list->Find(&father,&sun,n))
	{
		if (mainList != NULL)
		{
			if (!NextInList(mainList,sun->name.String()))	// si on ne trouve pas le fils,
				NextInList(mainList,father->name.String());	// on cherche le père
		}
		
		if (linksList != NULL)
			NextInList(linksList,sun->name.String());		// là on ne cherche que le fils
	}
	m_DontUpdateURL = false;
	((MainWindow*)Window())->GoTo(n);
}
		
// cette fonction statique est utilisée par la suivante, BrowseView::NextInList
static bool iterate(BListItem *item, void *v)
{
	BrowseView *bw = (BrowseView*)v;
	
	if (bw->myStr == ((BStringItem*)item)->Text())
	{
		if (!((BrowseView*)v)->notYet)
		{
			bw->myItem = item;
			return true;
		}
		else
			((BrowseView*)v)->notYet = (bw->myItem != item);
	}
	return false;
}

bool BrowseView::NextInList(BOutlineListView *list, const char *nameToSearch, BListItem *afterThis)
{
	myItem = afterThis;
	notYet = (afterThis!=NULL);
	myStr = nameToSearch;
	
	list->FullListDoForEach(&iterate,(void*)this);
	
	if ((myItem == NULL) || (myItem == afterThis))
	{		// un item suivant n'a pas été trouvé
		if (afterThis == NULL)
			// ben ya vraiment rien
			return false;
		
			// on peut peut être en trouver un avant
		myItem = NULL;
		list->FullListDoForEach(&iterate,(void*)this);
		
		if ((myItem == NULL) || (myItem == afterThis))
			// ça veut vraiment pas!
			return false;
	}
	
	// on a donc un nouvel item à sélectionner		
	BListItem *iterate = list->Superitem(myItem), *oldIterate = NULL;
	while (iterate != NULL)
	{
		iterate->SetExpanded(true);

		oldIterate = iterate;			
		iterate = list->Superitem(iterate);
	}

	// ce petit truc de Collapse/Expand permet d'être sûr que la outlinelist est mise à jour
	// j'ai fait comme ça parce que BOutlineListView::Expand() ne marche pas sur un item
	// qui n'est pas encore visible. BListItem::SetExpanded marche mais ne met pas à jour
	// l'affichage
	if (oldIterate != NULL)
	{
		list->Collapse(oldIterate);
		list->Expand(oldIterate);
	}
	
	// maintenant l'item devrait être visible
	unsigned int n = list->IndexOf(myItem);
	
	if (n<0)
		return false;	// ben non!
	list->Select(n);
	list->ScrollToSelection();
	return true;
}

void BrowseView::FindInIndex(bool select, const char *s)
{
	if (s==NULL)
		s = searchTextView->Text();
	BString str(s);
	
	if(!containsSearch)
	{
		unsigned int nL = linksList->CountItems();
	
		for (unsigned int i=0; i<nL; i++)
		{
			BStringItem *it = (BStringItem*)linksList->ItemAt(i);
			if (str.ICompare(it->Text()) <= 0)
			{
				BRect r = linksList->ItemFrame(i);
				linksList->ScrollTo(r.LeftTop());
			
				if (select)
					linksList->Select(i);
				break;
			}
		}
	}
	else
	{
		tasks->GiveUp();
		linksList->MakeEmpty();

		if (str == "")
			return;

		tasks->DoFindInIndex(str,this,select);			
	}
}

void BrowseView::BusyOn()
{
	be_app->SetCursor(clockMouse);
}

void BrowseView::BusyOff()
{
	be_app->SetCursor(B_HAND_CURSOR);
}

void BrowseView::ChangeInfo(const char *text)
{
	if ((infoText != NULL) && (lastInfo != text))
	{
		Window()->Lock();
		infoText->SetText(text);
		Window()->Unlock();
		lastInfo = text;
	}
}

LinkItem::LinkItem(const char *label,unsigned int l, uint32 level, bool expanded)
	: BStringItem(label,level,expanded)
{
	link = l;
}

LinkList::LinkList(BRect frame,const char *name,BrowseView *bw,list_view_type t, uint32 re, uint32 fl)
	: BOutlineListView(frame,name,t,re,fl)
{
	browseView = bw;
}

void LinkList::SelectionChanged()
{
	LinkItem *item = (LinkItem*)ItemAt(CurrentSelection());
	
	if ((item != NULL) && !(browseView->m_DontUpdateURL))
		((MainWindow*)Window())->GoTo(item->link);
}

void LinkList::MouseMoved(BPoint point,uint32 transit,const BMessage*)
{
	LinkItem *item = (LinkItem*)ItemAt(IndexOf(point));
	
	if (item != NULL)
	{
		if (transit != B_EXITED_VIEW)
			browseView->ChangeInfo(item->info);
		else
			browseView->ChangeInfo(B_EMPTY_STRING);
	}
}

// cette fonction est là pour appeler SelectionChanged même si l'objet sur lequel on clique
// est déjà sélectionné
void LinkList::MouseDown(BPoint point)
{
	LinkItem *item = (LinkItem*)ItemAt(IndexOf(point));
	
	if ((item != NULL) && (item->IsSelected()))
		item->Deselect();
	
	BOutlineListView::MouseDown(point);
}

LinkItem2::LinkItem2(const char *label,HappyLink *l, uint32 level, bool expanded)
	: BStringItem(label,level,expanded)
{
	link = l;
}

LinkList2::LinkList2(BRect frame,const char *name,list_view_type t,BrowseView *bw, uint32 re, uint32 fl)
	: BOutlineListView(frame,name,t,re,fl)
{
	browse = bw;
}

void LinkList2::SelectionChanged()
{
	LinkItem2 *item = (LinkItem2*)ItemAt(CurrentSelection());
	
	if (item != NULL)
		if ((!browse->NewLinksList(item->link)) && !(browse->m_DontUpdateURL))
			// si la liste de liens est vide, on ouvre le lien du père
			((MainWindow*)Window())->GoTo(item->link->link);
}