/*
* BeHappy
*
* gestion d'un index de liens HTML
*
*/

#ifndef HAPPYLINK_H
#define HAPPYLINK_H

#include <String.h>

class HTMLProject;
class HTMLLabel;
class BDataIO;
class BList;

typedef enum { SORT_ALL, SORT_PARENTS_ONLY, SORT_KIDS_ONLY } SortType;

class HappyLink
{
public:
	HappyLink(HTMLProject *proj,const char *name,HTMLLabel *link,const char *info = NULL);
	HappyLink(BDataIO*);
	HappyLink(HappyLink&);
	HappyLink();
	~HappyLink();
	
	void UpdateLink(HTMLProject *proj,HTMLLabel *link);
	status_t Archive(BDataIO*) const;
	
	void AddChild(HappyLink*);
	void AddChildSorted(HappyLink*,SortType=SORT_ALL);
	bool RemoveChild(HappyLink*);	// true si le fils a été enlevé
	void Dump(unsigned int level=0);
	HappyLink *Search(const BString &nameToSearch, unsigned int generation = 1000, BList *list=NULL, bool onlyKids = true);
		// generation: profondeur de recherche (= nb de générations à fouiller+1. Donc 2 est le minimum pour ne chercher que les enfants du happylink)
		// list: si différent de NULL, pointeur vers une liste de tous les happylinks trouvée. Dans ce cas, la fonction rend toujours NULL
		// onlyKids: ne cherche pas dans les frères du happylink

	bool Find(HappyLink **father,HappyLink **son, unsigned int linkNum,bool onlyKids=true);
		// retrouve un HappyLink et son père à partir d'un numéro
		
	unsigned int CountItems();
	
	BString name;
	BString info;
	unsigned int link;
	HappyLink *next,*child;
};


#endif //HAPPYLINK_H
