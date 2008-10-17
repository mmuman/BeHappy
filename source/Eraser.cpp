// Looper qui fait le ménage (vide les listes, par exemple) en parallèle
// avec les autres threads

#include "Eraser.h"
#include <OutlineListView.h>
#include <ScrollView.h>
#include <Message.h>
#include <ListItem.h>

#define ERASER_OLV 'Eolv'
#define EO_OLV "BOutlineListView"
#define EO_SV "BScrollView"

Eraser::Eraser()
	: BLooper("Eraser",B_LOW_PRIORITY)

{
	Run();
}

void Eraser::KillList(BOutlineListView *olv, BScrollView *sv)
{
	BMessage message(ERASER_OLV);
	message.AddPointer(EO_OLV,olv);
	message.AddPointer(EO_SV,sv);
	
	PostMessage(&message);
}

void Eraser::MessageReceived(BMessage *message)
{
	switch(message->what)
	{
	case ERASER_OLV:
	{
		BOutlineListView *olv;
		BScrollView *sv;
		
		message->FindPointer(EO_OLV,(void**)&olv);
		message->FindPointer(EO_SV,(void**)&sv);
		
		// on vérifie la validité du message avant d'effacer tout
		if ((olv != NULL) && (sv != NULL))
		{
			/// on parcourt tous les éléments pour les effacer
			int32 nit = olv->FullListCountItems();
			while (nit>0)
				delete olv->RemoveItem(--nit);
	
			/// et enfin on vire la ScrollView		
			delete sv;
		}
		
		// vala!!
	}
	
	default:
		BLooper::MessageReceived(message);
	}
}