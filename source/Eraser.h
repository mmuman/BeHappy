// Looper qui fait le ménage (vide les listes, par exemple) en parallèle
// avec les autres threads

#include <Looper.h>
class BOutlineListView;
class BScrollView;

class Eraser : public BLooper
{
public:
	Eraser();
	
	void MessageReceived(BMessage*);
	
	// Vide la liste outlinelistview.
	// On doit fournir la scrollview qui va avec. Elle doit être enlevée de la
	// fenetre avant l'appel de cette fonction
	void KillList(BOutlineListView*, BScrollView*);
	// L'élimination n'est pas terminée après l'appel de cette fonction, elle
	// n'a peut-être même pas commencé.
};
