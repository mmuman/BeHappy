/*
*   BeHappy
*
* fenêtre d'info sur le progrès de l'update
*
*
* version DK
*/

#ifndef INFOWINDOW_H
#define INFOWINDOW_H

#include <Window.h>

class InfoWindow : public BWindow
{
public:
	InfoWindow(BRect,BWindow*);
	~InfoWindow();
	
	void MessageReceived(BMessage *);
	
private:
	BStringView *line1;
	BStringView *line2;
	BStatusBar *bar;
};

#endif //INFOWINDOW_H