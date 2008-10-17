/*
* OpenGL specs Add-on for BeHappy 
*
* (c) 1999 Sylvain Tertois
*
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
const char *projectName="OpenGL specs";

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
	static unsigned int config[] = { 0, BW_SEARCHTXTV|BW_CONTAINS };
	return(config);
}

// name of main html file
const char *mainFileName="glspec.html";

// Put here the code to find the documentation path
// if you return false, a window will be opened asking for the path
bool AddOn::FindHome()
{
	// search for the file glspec.html
	BQuery myQuery;
	BVolumeRoster roster;
	
	BVolume vol;
	roster.GetBootVolume(&vol);
	myQuery.SetVolume(&vol);
	BString pred = "name=";
	pred << mainFileName;
	myQuery.SetPredicate(pred.String());
	myQuery.Fetch();
	BEntry myEntry;
	if (myQuery.GetNextEntry(&myEntry) == B_OK)
	{
		BDirectory dir;
		myEntry.GetParent(&dir);
		SetHome(BDirectory(dir));
	}
	else
		SetHome(BDirectory("/boot/home"));
	return false;
}

// The biggest part: put here your code to build the indexes
// don't use the HappyLink* (yet)
void AddOn::Update(HappyLink*)
{
	// first lets clean the old indexes if they're there
	Clean();
	
	// now lets go!! put your code here
	HTMLLabel *mainLabel = new HTMLLabel(this,mainFileName,"","Index");
		// the file name "index.html" up there should be the real name of the html file you want
		// to see when your Add-On is loaded.
		
	happyList = new HappyLink(this,projectName,mainLabel);
	
	// you must put at least an index or BeHappy will crash
	// and that way I can show you how this is done ;-)
	summary = new HappyLink(this,"Summary",mainLabel);
	index = new HappyLink(this,"Index",mainLabel);
	
	// add the index to the happy list:
	happyList->AddChild(summary);	// this can be done before or after you fill in the index
	happyList->AddChild(index);

	// Open the main html file
	HTMLFile *mainFile = mainLabel->GetFile();
	
	// Parse
	if (mainFile->Search() != B_OK)
	{
		BAlert *myAlert = new BAlert("BeHappy: OpenGL","Error while opening index file: glspec.html",
			"OK",NULL,NULL,B_WIDTH_AS_USUAL,B_STOP_ALERT);
		myAlert->Go();
		return;
	}
	BList links(20);
	mainFile->GetLists(0,1,&links,NULL);
	
	// set up the status bar
	NumPrint(links.CountItems());
	Print("Updating the Summary...");
	
	// Scan for chapters
	// We'll allways look at the first item in the list, and remove it when
	//  it has been included in the summary
	
	// first go to chapter 1
	while(links.CountItems()>0)
	{
		HTMLLabel *label = (HTMLLabel*)links.ItemAt(0);
		
		char sc = label->LabelText()[0];
		if (sc != '1')
		{
			links.RemoveItem((int32)0);
			Print(NULL,"");	// update the status bar
		}
		else
		{
			// we've found the chapter... break the waiting loop
			break;
		}
	}

	// and add the chapters
	AddItems(links,0,summary);
}

// Add the items of a chapter. This function can be called recursively
void AddOn::AddItems(BList &links,unsigned int level,HappyLink *chapter)
{
	HappyLink *lastOne = NULL;	// pointer to the last added item
	
	while(links.CountItems() > 0)
	{
		HTMLLabel *labelToAdd = (HTMLLabel*)links.ItemAt(0);
		BString labelName(labelToAdd->LabelText());
		BString labelNum;
		
		// if this is the index, return
		if (labelName.IFindFirst("Index")==0)
			return;

		// extract the number (ex: 1.2.1) of the label name
		int n = labelName.FindFirst(' ');
		if (n<=0)
		{	// if we don't find a space, this is a bad title
			links.RemoveItem((int32)0);
			continue;
		}
		labelName.CopyInto(labelNum,0,n);

		// find the depth level of this label. It's the number of points in the title
		unsigned int labelLevel=0;
		int i=-1;
		while((i=labelNum.FindFirst('.',i+1)) >= 0)
			labelLevel++;
		
		// cut the number from the name
//		if ((labelLevel>0) || (n<3))
//			labelName.Remove(0,n+1);

		// Now for the fun: if labelLevel==0 that can mean two things:
		// - it is a big chapter (ex.: 1 Introduction)
		// - it is a thing without any number (ex.: Floating-Point Computation)
		// I'll assume that if n<3 we are in the first case, and labelLevel will
		// stay at 0. But if n>=3, I'll put the item back at his level.
		if ((labelLevel == 0) && (n>=3))
			labelLevel = level;
		else 		
		// cut the number from the name
			labelName.Remove(0,n+1);
		
		// if the level is the same that the current level, just add the item
		if (labelLevel == level)
		{				// if there's no point we'll say the level is good
			HappyLink *item = new HappyLink(this,labelName.String(),labelToAdd);
			chapter->AddChild(item);
			lastOne = item;
			
			HappyLink *itemCopy = new HappyLink(*item);
			index->AddChildSorted(itemCopy);
			
			links.RemoveItem((int32)0);

			Print(NULL,"");	// update the status bar
		}
		else if (labelLevel>level)
		{
			// we must put these labels one level higher
			AddItems(links,labelLevel,lastOne);
		}
		else
		{
			// it is the end of this section
			return;
		}			
	}
}

// About box
void AddOn::About() const
{
	BAlert *info = new BAlert("BeHappy:OpenGL Specifications","OpenGL Specifications...\n(c) 1992-1997 Silicon Graphics, Inc.\nAdd-on by Sylvain Tertois","OK");
	info->Go();
}