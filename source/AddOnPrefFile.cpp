/*
* BeHappy
*
*
* Gestion des préférences liées à chaque add-on
*
*/

#include <Message.h>
#include <StorageKit.h>
#include "AddOnPrefFile.h"
#include "BeHappy.h"

AddOnPrefFile::AddOnPrefFile(const char *nm)
{
	name = nm;
	BEntry fileEntry;
	if (((BeHappy*)be_app)->prefsDir->FindEntry(nm,&fileEntry) == B_OK )
	{
		BFile prefFile(&fileEntry,B_READ_ONLY);
		
		archive = new BMessage('BhAo');
		archive->Unflatten(&prefFile);
	}
	else
		archive = NULL;
}

AddOnPrefFile::~AddOnPrefFile()
{
	if (archive == NULL)
		return;
		
	BFile myFile;
	((BeHappy*)be_app)->prefsDir->CreateFile(name.String(),&myFile);
	
	archive->Flatten(&myFile);	
}

BMessage *AddOnPrefFile::GetIndex()
{
	if (archive == NULL)
		return NULL;
		
	BMessage *ret=new BMessage('BhAo');
	
	if (archive->FindMessage("Index",ret) != B_OK)
	{
		delete ret;
		ret = NULL;
	}
	
	return ret;
}


BMessage *AddOnPrefFile::GetPrefs()
{
	if (archive == NULL)
		return NULL;
		
	BMessage *ret=new BMessage('BhAo');
	
	if (archive->FindMessage("Preferences",ret) != B_OK)
	{
		delete ret;
		ret = NULL;
	}
	
	return ret;
}

void AddOnPrefFile::SetIndex(const BMessage *index)
{
	if (archive == NULL)
		archive = new BMessage('BhAo');
		
	archive->RemoveName("Index");
	archive->AddMessage("Index",index);
}

void AddOnPrefFile::SetPrefs(const BMessage *preferences)
{
	if (archive == NULL)
		archive = new BMessage('BhAo');
		
	archive->RemoveName("Preferences");
	archive->AddMessage("Preferences",preferences);
}

