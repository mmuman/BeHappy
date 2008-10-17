/*
* Add-on BeBook pour BeHappy
*
* (c) 1999 Sylvain Tertois
*
*/

#ifndef BHBEBOOK_H
#define BHBEBOOK_H

#include "HTMLProject.h"

class BHBeBook : public HTMLProject
{
public:
	BHBeBook(BMessage*);
	BHBeBook();
	
	bool FindHome();
	void Update(HappyLink*);
	void DoKit(HTMLLabel *l, HappyLink *happyKit, HappyLink *others, bool addToKits = true);
	void DoClass(HTMLLabel *lc, HappyLink *happyClass, bool doInheritance = true);
		// quand doInheritance est false:
		// le fichier est considéré comme une classe, et pas un topic
		// aucune recherche n'est faite pour l'héritage
		// pour l'index "By Inheritance", les méthodes sont ajoutées à l'objet
		// pointé par m_LatestByInheritanceClass, c-à-d la dernière classe qui
		// a été mise dans l'index "By inheritance"
		
	const unsigned int *GiveConfig() const;

	bool TestMethodName(const char *name);	// vérifie la validité d'un nom de méthode
	void ClearStr(BString&);		// supression des espaces de début et de fin

	void About() const;
private:
	HappyLink *byKit,*byInheritance,*functionsIndex,*topicsIndex;
	
	bool m_NewBeBook;
		// true si c'est in nouveau BeBook (>= Oct. 2000)
	
	// pointeurs vers la dernière classe ajoutée aux index. ça sert pour les classes qui
	// sont décrites dans plusieurs fichiers (ex. BeBook)
	HappyLink *m_LatestByInheritanceClass;	// dans l'index "par héritage"
};
#endif //BHBEBOOK_H