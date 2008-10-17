/*
* BeHappy
*
* fenêtre principale du parser
*/

#include <Window.h>
class ColumnListView;
class BFilePanel;

class ParserWindow : public BWindow
{
public:
	ParserWindow();
	~ParserWindow();
	
	bool QuitRequested();

	void MessageReceived(BMessage*);
	
	void Parse();

private:
	BTextControl *fileName;
	ColumnListView *beacons,*labels;
	BFilePanel *panel;
};