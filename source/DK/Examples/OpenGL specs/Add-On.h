/*
* OpenGL specs Add-on for BeHappy 
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
	void AddItems(BList &links,unsigned int level,HappyLink *chapter);

	HappyLink *summary,*index;
};

#endif //ADDON_H