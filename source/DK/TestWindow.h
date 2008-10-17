/*
*  BeHappy Test
*
* Fenêtre de test
*
*/

#ifndef TESTWINDOW_H
#define TESTWINDOW_H

#include <InterfaceKit.h>
#include <String.h>
#include <kernel/OS.h>
class HTMLProject;
class HappyLink;
class InfoWindow;

class LinkItem : public BStringItem
{
public:
	LinkItem(const char*,unsigned int link,uint32 level=0, bool expanded = true);
	
	unsigned int link;
};

class LinkList : public BOutlineListView
{
public:
	LinkList(BRect,const char*, list_view_type lt,uint32 res=B_FOLLOW_ALL,uint32=B_WILL_DRAW|B_NAVIGABLE|B_FRAME_EVENTS);
	
	void SelectionChanged();
};	

class TestWindow : public BWindow
{
public:
	TestWindow(HTMLProject*,const BString&);
	~TestWindow();
	
	void MessageReceived(BMessage*);
	bool QuitRequested();

	HTMLProject *proj;

private:
	void UpdateList(BStringItem*,HappyLink*);

	LinkList *list;
	BStringView *string;
	InfoWindow *iw;
	thread_id updateThread;
	bool paused;
};

#endif //TESTWINDOW_H