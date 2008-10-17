/*
* BeHappy Documentation Add-on 
*
* (c) 1999 Sylvain Tertois
*
*/

#ifndef ADDON_H
#define ADDON_H

#include "HTMLProject.h"

class AddOn : public HTMLProject
{
public:
	AddOn(BMessage*);
	AddOn();
	
	void Update(HappyLink*);
	const unsigned int *GiveConfig() const;
	bool FindHome();

	void About() const;

private:
	void NewFile(HTMLFile *file, HappyLink *list);
	unsigned int CountHTMLFiles(BDirectory &dir);

	HappyLink *classes,*topics;
};

#endif //ADDON_H