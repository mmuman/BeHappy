/*
*  BeHappy
*
* Remplaçant d'OutlineListView optimisé pour BeHappy
*
*/

#ifndef HAPPYLISTVIEW_H
#define HAPPYLISTVIEW_H

#include <View.h>
#include <Point.h>
#include <List.h>

class BListItem;
class BPoint;

class HappyListView : public BView
{
public:
	HappyListView(BRect frame, const char *name, uint32 type=0,
		uint32 resizing_mode = B_FOLLOW_LEFT | B_FOLLOW_TOP,
		uint32 flags = B_WILL_DRAW | B_FRAME_EVENTS | B_NAVIGABLE);
	~HappyListView();
	
	bool AddItem(BListItem *item, int index=0);
	bool AddUnder(BListItem *item, BListItem *superItem);
	BListItem *ItemAt(int index);
	BListItem *RemoveItem(int index);
	int IndexOf(BListItem *myItem);
	int IndexOf(BPoint point);
	
	void MakeEmpty();
	bool IsEmpty();
	
	void Select(int index);
	int CurrentSelection();
	void ScrollToSelection();
	
	int FullListCountItems();
	void FullListDoForEach(bool (*)(BListItem *, void *), void *);
	BListItem *Superitem(BListItem *);
	int CountItems();
	
	void Collapse(BListItem *);
	void Expand(BListItem *);
	
	BRect ItemFrame(int i);

	virtual void FrameResized(float width, float height);
	
private:
	BList m_ItemList;
	int m_CurrentSelection;
};

#endif