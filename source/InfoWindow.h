/*
*   BeHappy
*
* fenêtre d'info sur le progrès de l'update
*
*
*/

#ifndef INFOWINDOW_H
#define INFOWINDOW_H

#include <Window.h>
class MainWindow;
class BStringView;
class BStatusBar;

class InfoWindow : public BWindow
{
public:
	InfoWindow(BRect);
	~InfoWindow();
	
	void MessageReceived(BMessage *);
	
private:
	BStringView *line1;
	BStringView *line2;
	BStatusBar *bar;
};

#endif //INFOWINDOW_H
