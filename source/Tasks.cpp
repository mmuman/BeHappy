// Looper qui fait certaines tâches en parallèle avec la fenêtre
// (pour l'instant: update d'un projet, et recherche dans l'index)

#include "Tasks.h"
#include "MainWindow.h"
#include "BrowseView.h"
#include "HappyLink.h"

#include <stdio.h>

Tasks::Tasks(MainWindow *w)
	: BLooper("Tasks")
{
	win = w;
	giveUp = false;
	
	Run();
}

void Tasks::MessageReceived(BMessage *message)
{
	giveUp = false;
	switch(message->what)
	{
		case 'Upda':
			job.Lock();
			win->UpdateProject();
			job.Unlock();
		break;
		
		case 'FIIn':
		{
			const char *s;
			BrowseView *caller;
			bool select;
			
			if ((message->FindString("string",&s) == B_OK) &&
				(message->FindPointer("caller",(void**)&caller) == B_OK) &&
				(message->FindBool("select",&select) == B_OK))
			{
				BString str(s);
				FindInIndex(str,caller,select);
			}		
		}
		break;
		
		default:
			BLooper::MessageReceived(message);
		break;
	}
}

void Tasks::GiveUp()
{
	giveUp=true;

	// on attend que la tâche soit réellement terminée
	job.Lock();
	job.Unlock();
}

void Tasks::DoProjectUpdate()
{
	GiveUp();
	
	PostMessage('Upda');
}

void Tasks::DoFindInIndex(BString &str,BrowseView *caller, bool select)
{
	BMessage message('FIIn');
	message.AddString("string",str.String());
	message.AddPointer("caller",caller);
	message.AddBool("select",select);
	
	PostMessage(&message);
}

void Tasks::FindInIndex(BString &str,BrowseView *caller, bool select)
{
	job.Lock();
	
	// on va chercher dans la happylist tous les liens qui contiennent la
	// chaîne de charactères, et on les met dans la liste linksList
	LinkItem *listItem = NULL;
	for (HappyLink *p=caller->top->child; p != NULL; p = p->next )
	{
		if (p->name.IFindFirst(str) >=0)
		{
			// oui, on doit ajouter cet item dans la liste
			listItem = new LinkItem(p->name.String(),p->link,0,false);
			listItem->info = p->info.String();
		
			// alors là, c'est un peu compliqué: pour ajouter un élément dans
			// la liste, on doit Locker le Looper. Mais si BrowseView est en
			// train d'abandonner la recherche, (avec Tasks::GiveUp), le Looper
			// est déjà Locké, et il attend la fin du job, d'où deadlock.
			// donc j'utilise plutôt LockLooperWithTimeout. S'il y a timeout
			// mais pas abandon du boulot demandé (giveUp à false), je
			// réessaie de locker le looper. C'est le rôle du while avec le
			// booléen go
			bool go = true;
			while(go)
				if (caller->LockLooperWithTimeout(100000) == B_OK)
				{
					if (caller->linksList != NULL)
			 			caller->linksList->AddItem(listItem,0);
					caller->UnlockLooper();
					go = false;
				}
				else
					if (giveUp)
						go = false;
		}

		if (giveUp)
			break;
	}
	
	if (select & (listItem != NULL))
	{
		caller->LockLooper();
		 if (caller->linksList != NULL)
		 	caller->linksList->Select(0);
		caller->UnlockLooper();
	}
	
	job.Unlock();
}