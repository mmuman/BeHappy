// Looper qui fait certaines tâches en parallèle avec la fenêtre
// (pour l'instant: update d'un projet, et recherche dans l'index)


#include <Looper.h>
#include <Locker.h>
#include <String.h>

class MainWindow;
class BrowseView;

class Tasks : public BLooper
{
public:
	Tasks(MainWindow*);
	
	void MessageReceived(BMessage *message);
	void GiveUp();
	
	void DoProjectUpdate();	// fait un update du projet en cours de la fenêtre
	void DoFindInIndex(BString &str, BrowseView *caller, bool select);
			// met à jour l'index dela fenêtre en mode "contains"
	
private:
	void FindInIndex(BString &str, BrowseView *caller, bool select);

	MainWindow *win;
	BLocker job;
	bool giveUp;
};
