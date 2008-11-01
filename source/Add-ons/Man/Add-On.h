/*
* Man pages Add-on 
*
* (c) 2003 François Revol
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
	const char *SectionName(const char *n);

	HappyLink *classes,*topics;
};

#endif //ADDON_H
