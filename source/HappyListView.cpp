/*
*  BeHappy
*
* Remplaçant d'OutlineListView optimisé pour BeHappy
*
*/

#include "HappyListView.h"

HappyListView::HappyListView(BRect frame, const char *name, uint32 type,
		uint32 resizing_mode, uint32 flags);
		: BView(frame,name,resizing_mode,flags)
{
	m_CurrentSelection=-1;
}

HappyListView::~HappyListView();
{
}

bool HappyListView::AddItem(BListItem *item, int index);
{
	m_ItemList.AddItem(item,index);
	
	Invalidate();
	
	return true;
}

bool HappyListView::AddUnder(BListItem *item, BListItem *superItem);
{
	AddItem(item,m_ItemList.IndexOf(superItem)+1);
	
	return true;
}

BListItem *HappyListView::ItemAt(int index);
{
	return (BListItem*)m_ItemList.ItemAt(index);
}

BListItem *HappyListView::RemoveItem(int index);
{
	BListItem *item = (BListItem*)m_ItemList.RemoveItem(index);
	Invalidate();
	
	return item;
}

int HappyListView::IndexOf(BListItem *myItem);
{
	return m_ItemList.IndexOf(item);
}

int HappyListView::IndexOf(BPoint point);
{
	// on récupère la hauteur d'un élément de la liste
	BListItem *l_FirstItem = (BListItem*)m_ItemList.ItemAt(0);
	if (l_FirstItem == NULL)
		return -1;
	
	int l_Index = (point.y+Bounds().top) / l_FirstItem->Height();
	if ((l_Index < 0) || (l_Index >= m_ItemList.CountItems()))
		return -1;
	else
		return l_Index;
}

void HappyListView::MakeEmpty();
{
	m_ItemList.MakeEmpty()
	
	Invalidate();
}

bool HappyListView::IsEmpty();
{
	return m_ItemList.IsEmpty();
}

void HappyListView::Select(int index);
{
	BListItem *l_Item = ItemAt(m_CurrentSelection);
	if (l_Item != NULL)
		l_Item->Deselect();
	
	l_Item = ItemAt(index);
	if (l_Item != NULL)
	{
		l_Item->Select();
		m_CurrentSelection = index;
	}
	
	Invalidate();
}

int HappyListView::CurrentSelection();
{
	return m_CurrentSelection;
}

void HappyListView::ScrollToSelection();
{
}

int HappyListView::FullListCountItems();
{
	return m_ItemList.CountItems();
}

void HappyListView::FullListDoForEach(bool (*func)(BListItem *, void *), void *data);
{
	
}

BListItem *HappyListView::Superitem(BListItem *);
{
}

int HappyListView::CountItems();
{
}

void HappyListView::Collapse(BListItem *);
{
}

void HappyListView::Expand(BListItem *);
{
}

BRect HappyListView::ItemFrame(int i);
{
}
