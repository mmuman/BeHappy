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

#include "Add-On.h"
#include "HappyLink.h"
#include "HTMLFile.h"
#include "BHAdd-ons.h"
#include <Alert.h>

// don't touch these!
// these two variables are used by BeHappy to be shure it's compatible with the add-on
const uint16 BHVersion = BHAO_VERSION;
const uint16 BHVLastCompatible = BHAO_LCVERSION;


HTMLProject *InstantiateProject(BMessage *archive)
{
	if (archive==NULL)
		return new AddOn;
	else
		return new AddOn(archive);
}

// Put Add-On name here
const char *projectName="Sample AddOn";

AddOn::AddOn(BMessage *archive)
	: HTMLProject(archive)
{
}

AddOn::AddOn()
{
}

// Give the configuration of the different indexes
const unsigned int *AddOn::GiveConfig() const
{
	static unsigned int config[] = { 0 };
	return(config);
}

// Put here the code to find the documentation path
// if you return false, a window will be opened asking for the path
bool AddOn::FindHome()
{
	// here the directory is placed to "/boot/home"
	// replace this with your code
	SetHome(BDirectory("/boot/home"));
	return true;
}

// The biggest part: put here your code to build the indexes
// don't use the HappyLink* (yet)
void AddOn::Update(HappyLink*)
{
	// first let's clean the old indexes if they're there
	Clean();
	
	// now let's go!! put your code here
	HTMLLabel *mainFile = new HTMLLabel(this,"index.html","","Index");
		// the file name "index.html" up there should be the real name of the html file you want
		// to see when your Add-On is loaded.
		
	happyList = new HappyLink(this,projectName,mainFile);
	
	// you must put at least an index or BeHappy will crash
	// and that way I can show you how this is done ;-)
	HappyLink *index = new HappyLink(this,"Index",mainFile);
	
	// add the index to the happy list:
	happyList->AddChild(index);	// this can be done before or after you fill in the index
}

// About box
void AddOn::About() const
{
	BAlert *info = new BAlert("BeHappy:AddOn","Sample AddOn","OK");
	info->Go();
}