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


// il suffit d'avoir un objet global de la classe Preferences, et on peut
// y accéder quand on veut
#ifndef PREFERENCES_H
#define PREFERENCES_H

#include <SupportKit.h>

class Preferences
{
public:
	Preferences(const char *path);		// chemin du fichier de préférences
	~Preferences();

	inline status_t InitCheck() { return initCheck; }
	
// enregistrer une préférence
	void AddPref(BMessage *pref,const char *name,unsigned int index = 0);
// retirer une préférence (NULL si non trouvée)
	BMessage *FindPref(const char *name,unsigned int index = 0);

// si la préférence est une chaine de charactères
/// FindString rend def si aucune chaine n'a été trouvée
/// l'objet BString créé par FindString devient la propriété de l'appelant
	void AddString(const char *str,const char *name,unsigned int index = 0);
	BString *FindString(const char *def,const char *name,unsigned int index = 0);

// si la préférence est un entier
/// FindInt rend def si aucun entier n'a été trouvé
	void AddInt(int intToAdd,const char *name,unsigned int index = 0);
	int FindInt(int def,const char *name,unsigned int index = 0);
	
// pour les fenêtres (mémorisation de la place et de la taille)
/// a mettre avant Show() dans le constructeur (you=this)
	void SizeWindow(const char *name,BWindow *you);
/// a mettre dans le destructeur
	void CloseWindow(BWindow *you);

private:
	BString path;
	status_t initCheck;
	BMessage *prefs;
	BList *windowNameList;
	BList *windowIndexList;
	BList *windowThisList;
	
	sem_id semaphore;
};

#endif //PREFERENCES_H