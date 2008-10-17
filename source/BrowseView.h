/*
*  BeHappy
*
* View avec la liste des trucs à afficher
*
*/

#ifndef BROWSEVIEW_H
#define BROWSEVIEW_H

#include <InterfaceKit.h>
#include <String.h>

#include <OutlineListView.h>

// options de la browseview
#define BW_SECONDLIST	0x01
#define BW_NEXTBUTTON	0x02
#define BW_SEARCHTXTV	0x04
#define BW_CONTAINS		0x08
#define BW_INFOTEXT		0x10

class HappyLink;
class Mover;
class BrowseView;
class MainWindow;

/* LinkItem et LinkList: une liste qui est associée à un lien */
/* (celle du dessous, ou celle qui est toute seule) */
class LinkItem : public BStringItem
{
public:
	LinkItem(const char*,unsigned int link,uint32 level=0, bool expanded = true);
	
	unsigned int link;
	const char *info;
};

class LinkList : public BOutlineListView
{
public:
	LinkList(BRect,const char*,BrowseView*,list_view_type lt,uint32 res=B_FOLLOW_ALL,uint32 flags=B_WILL_DRAW|B_NAVIGABLE|B_FRAME_EVENTS);
	
	void MouseMoved(BPoint,uint32,const BMessage*);
	void MouseDown(BPoint);
	void SelectionChanged();

private:
	BrowseView *browseView;
};	

/* LinkItem2 et LinkList2: une liste qui modifie le contenu de la liste ci-dessus */
/* (celle du dessus, quand elle est là) */
class LinkItem2 : public BStringItem
{
	public:
	LinkItem2(const char*,HappyLink *link,uint32 level=0,bool expanded = true);
	
	HappyLink *link;
};

class BrowseView;
class LinkList2 : public BOutlineListView
{
public:
	LinkList2(BRect,const char*,list_view_type lt,BrowseView *bw,uint32 res=B_FOLLOW_ALL,uint32 flags=B_WILL_DRAW|B_NAVIGABLE|B_FRAME_EVENTS);
	
	void SelectionChanged();

private:
	BrowseView *browse;
};

/* la BrowseView */
class Tasks;
class BrowseView : public BView
{
public:
	BrowseView(BRect,Tasks *tasks);
	~BrowseView();

	void ResizeList(float delta);	
	void NewList(HappyLink *start,unsigned int mode);
	bool NewLinksList(HappyLink *start);	// rend true si la liste est non vide
	void Clear();

	void NextClicked();
	void ContainsClicked();
	
	bool NextInList(BOutlineListView *list, const char *nameToSearch, BListItem *afterThis = NULL);
		// sélectionne l'item dans la liste qui a 'nameToSearch' comme label
		// si afterThis est fourni, cherche en priorité après l'élément fourni
	void FindInIndex(bool select = false, const char *s=NULL);
		// met la liste en face de l'élément le plus proche de s
		// (contenu de searchTextView si s=NULL)
		// si select est true, sélectionne le premier élément trouvé
	void ChangeInfo(const char *text);
		
	int GetLinkNum();						// rend le numéro du lien actuellement sélectionné
	void SetLinkNum(int,HappyLink *list);	// le fixe
	
	float GetMoverPos();	// pour l'horizontal
	
	BListItem *myItem;	// utilisé par NextInList et iterate
	BString myStr;
	bool notYet;

	static void BusyOn();
	static void BusyOff();

public:
	HappyLink *top;		// premier happylink
	bool	m_DontUpdateURL;	// dit aux listes de ne pas mettre à jour l'URL (mis à false quand il y a des chances que SelectionChanged() soit appelée plusieurs fois de suite)
	
private:
	void UpdateList(LinkList *list,LinkItem *father,HappyLink *father2,bool onlyKids);
	void UpdateList(LinkList2 *list,LinkItem2 *father,HappyLink *father2);
		
	float bottomSpace;
	bool containsSearch;
	LinkList2 *mainList;
	LinkList *linksList;	
	BButton *nextButton;
	BTextControl *searchTextView;
	BCheckBox *containsView;
	BStringView *infoText;
	
	BString lastInfo;
	
	BScrollView *mainScroll,*linksScroll;
	Mover *horizontalMover;

	Tasks *tasks;	
	friend class Tasks;
	friend class MainWindow; 
};

#endif //BROWSEVIEW_H