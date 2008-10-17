/*
* BeHappy
*
*
* Fenêtre pour modifier le chemin de base de la doc HTML
*
*/

#include <Window.h>

class HTMLProject;
class BMessage;
class BTextControl;
class BFilePanel;

class PathWindow : public BWindow
{
public:
	PathWindow(HTMLProject *proj);
	
	void MessageReceived(BMessage*);

private:
	BMessage *replyToThis;
	BTextControl *path;
	
	BFilePanel *panel;
	
	HTMLProject *proj;
	
	typedef BWindow inherited;
};
