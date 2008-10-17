/*
***************************************
*                                     *
*         Laser Studio - BeOs         *
*                                     *
* Adapté par Trap#1 (Sylvain Tertois) *
* (1998-1999)                         *
*-------------------------------------*
*                                     *
*    Editeur - Debugger - Moniteur    *
*                                     *
*   classe générique de gestion de    * 
*            préférences              *
*                                     *
***************************************
*/


#include "Preferences.h"
#include <StorageKit.h>
#include <Message.h>
#include <kernel/OS.h>	// pour les sémaphores
#include <Window.h>

Preferences::Preferences(const char *p)
{
	windowNameList = new BList(10);
	windowIndexList = new BList(10);
	windowThisList = new BList(10);
	
	prefs = new BMessage('Pref');
	path = p;

// ouverture du fichier de préférences
	BFile prefFile(path.String(),B_READ_ONLY);
	initCheck = prefFile.InitCheck();
	
	if (initCheck == B_ENTRY_NOT_FOUND)
	{
		initCheck = B_OK;
		return;
	}

// chargement des préférences
	initCheck = prefs->Unflatten(&prefFile);

// création du sémaphore pour intervenir sur les préférences
	semaphore = create_sem(1,"preferences");
}

Preferences::~Preferences()
{
	BFile prefFile(path.String(),B_WRITE_ONLY | B_CREATE_FILE | B_ERASE_FILE);
	
	if (prefFile.InitCheck() != B_OK)
		return;
	
	acquire_sem(semaphore);
	prefs->Flatten(&prefFile);

	delete prefs;
	delete windowNameList;
	delete windowIndexList;
	delete windowThisList;
	
	delete_sem(semaphore);
}

void Preferences::AddPref(BMessage *pref,const char *name,unsigned int index)
{
	BString messageName=name;
// on ajoute un caractère à la fin pour inclure l'index dans le nom
	messageName += (char)(65+index);

	acquire_sem(semaphore);	
// si la préférence est encore là, on la vire
	prefs->RemoveName(messageName.String());

// puis on ajoute la nouvelle préférence
	prefs->AddMessage(messageName.String(),pref);
	release_sem(semaphore);
}

BMessage *Preferences::FindPref(const char *name,unsigned int index)
{
	BString messageName=name;
// on ajoute un caractère à la fin pour inclure l'index dans le nom
	messageName += (char)(65+index);

	acquire_sem(semaphore);
// extraction du message
	BMessage *pref = new BMessage('????');
	status_t error;
	
	error = prefs->FindMessage(messageName.String(),pref);
	release_sem(semaphore);
	
	if (error == B_OK)
		return pref;
	
	delete pref;
	return NULL;
}


void Preferences::AddString(const char *str,const char *name,unsigned int index)
{
	BMessage *pref = new BMessage('StrP');
	pref->AddString("str",str);
	
	AddPref(pref,name,index);
}

BString *Preferences::FindString(const char *def,const char *name,unsigned int index)
{
	BMessage *pref = FindPref(name,index);
	BString *result;
	const char *tmp;
	
	if ( (pref != NULL) && (pref->FindString("str",0,&tmp) == B_OK) )
		result = new BString(tmp);
	else
		result = new BString(def);

	delete pref;
		
	return result;
}

void Preferences::AddInt(int intToAdd,const char *name,unsigned int index)
{
	BMessage *pref = new BMessage('IntP');
	pref->AddInt32("int",intToAdd);
	
	AddPref(pref,name,index);
}

int Preferences::FindInt(int def,const char *name,unsigned int index)
{
	BMessage *pref = FindPref(name,index);
	int32 result;
	
	if ( (pref == NULL) || (pref->FindInt32("int",0,&result) != B_OK) )
		result = def;

	if (pref != NULL)
		delete pref;
		
	return result;
}
	
// pour les fenêtres (mémorisation de la place et de la taille)
/// a mettre avant Show() dans le constructeur (you=this)
void Preferences::SizeWindow(const char *name,BWindow *you)
{
// d'abord on trouve un index de fenêtre libre (inférieur à 5 quand même)
	unsigned int index,n = windowNameList->CountItems(),i;
	bool found = false;
	for (index = 0; !found && index<5; index++)
	{
		i=0;
		while ( (i<n) && !(( *((BString*)windowNameList->ItemAt(i)) == name) 
				&& (*(unsigned int*)(windowIndexList->ItemAt(i)) == index)))
			i++;
		if (i>=n)
			found = true;
	}
	
// on a pas trouvé d'index libre inférieur à 5, donc on ne retient rien
	if (found == false)
		return;
	
// on a l'index de la fenêtre... maintenant, on récupère la préférence associée
	BMessage *pref = FindPref(name,--index);
	
// et on redimensionne la fenêtre
	if (pref != NULL)
	{
		BRect windowFrame;
		if (pref->FindRect("rect",&windowFrame) == B_OK)
		{
			you->MoveTo(windowFrame.left,windowFrame.top);
			you->ResizeTo(windowFrame.Width(),windowFrame.Height());
		}
		delete pref;
	}

// puis enfin on ajoute la fenêtre à la liste interne
	BString *nm = new BString(name);
	windowNameList->AddItem(nm);
	windowIndexList->AddItem(new unsigned int(index));
	windowThisList->AddItem(you);		 
}

/// a mettre dans le destructeur
void Preferences::CloseWindow(BWindow *you)
{
// d'abord on recherche la fenêtre dans les listes
	int i = windowThisList->IndexOf(you);
	
// si elle n'est pas dans la liste, on ne sauve pas de paramètres...
	if (i<0)
		return;

// sinon, on extrait son nom et son index
	const char *name;
	unsigned int index;
	name = ((BString*)windowNameList->ItemAt(i))->String();
	index = *(unsigned int*)windowIndexList->ItemAt(i);

// création du message de préférences
	BMessage *pref = new BMessage('RecP');
	pref->AddRect("rect",you->Frame());

// et écriture dans les préférences
	AddPref(pref,name,index);

// finalement on purge les listes
	delete windowIndexList->ItemAt(i);
	delete windowNameList->RemoveItem(i);
	windowIndexList->RemoveItem(i);
	windowThisList->RemoveItem(i);
}

