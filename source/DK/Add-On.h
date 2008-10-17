/*
* BeHappy Add-on sample code 
*
* (c) 1999 Sylvain Tertois
*
* you can use this as a start for a custom add-on.
* just make a copy of the following files:
*  Add-On.h
*  Add-On.cpp
*  Add-On.proj
*  Add-On Test.proj
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
};

#endif //ADDON_H