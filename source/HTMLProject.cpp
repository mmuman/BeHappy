/*
*
*  Projet HTML
*
*/

#include "HTMLProject.h"
#include "HTMLFile.h"
#include "HappyLink.h"
#include "Version.h"

#include "BeHappy.h"

#include <stdio.h>
#include <Alert.h>

HTMLProject::HTMLProject()
{
	fileList = new BList(10);
	labelList = new BList(100);
	happyList = NULL;
	initOK = true;
}

const char hpD[]="Directory";
const char hpF[]="Files";
const char hpLn[]="Label names";
const char hpLf[]="Label files";
const char hpH[]="Happy List";
const char hpV[]="Version";

typedef struct _smallLabel
{
	unsigned int file;
	BString label;
} smallLabel;

HTMLProject::HTMLProject(BMessage *archive)
{
	fileList = new BList(10);
	labelList = new BList(100);
	happyList = NULL;
	initOK = false;

	uint16 archiveVersion;
	if (archive->FindInt16(hpV,(int16*)&archiveVersion) != B_OK)
		return;
	if ((archiveVersion < bhv_last_prefs) || (archiveVersion > bhv_application))
		return;
		
	entry_ref dirRef;
	if (archive->FindRef(hpD,&dirRef) != B_OK)
		return;
	filesDir.SetTo(&dirRef);
	
	const void *happyData;
	ssize_t dataSize;
	if (archive->FindData(hpH,B_RAW_TYPE,&happyData,&dataSize) != B_OK)
		return;
	BMemoryIO happyArchive(happyData,dataSize);
	happyList = new HappyLink(&happyArchive);
	
	unsigned int i=0;
	while(archive->HasString(hpF,i))
		fileList->AddItem(new HTMLFile(this,archive->FindString(hpF,i++)));
	
	i=0;
	while(archive->HasInt32(hpLf,i))
	{
		smallLabel *lab = new smallLabel;
		lab->label = archive->FindString(hpLn,i);
		lab->file = archive->FindInt32(hpLf,i++);
		labelList->AddItem(lab);
	}
	initOK = true;
}

HTMLProject::~HTMLProject()
{
	Clean();
	delete fileList;
	delete labelList;
	delete happyList;
}

BArchivable *HTMLProject::Instantiate(BMessage *archive)
{
	if (!validate_instantiation(archive, "HTMLPRoject"))
		return NULL;
	return new HTMLProject(archive);
}

status_t HTMLProject::Archive(BMessage *archive,bool deep)
{
	archive->AddInt16(hpV,bhv_application);	// version

	BEntry dirEntry;
	entry_ref dirRef;
	filesDir.GetEntry(&dirEntry);
	dirEntry.GetRef(&dirRef);
	
	archive->AddRef(hpD,&dirRef);

	BMallocIO happyArchive;
	happyList->Archive(&happyArchive);
	archive->AddData(hpH,B_RAW_TYPE,happyArchive.Buffer(),happyArchive.BufferLength());
	
	unsigned int n = fileList->CountItems();
	for (unsigned int i=0; i<n; i++)
		archive->AddString(hpF,((HTMLFile*)fileList->ItemAt(i))->name.String());
		
	n = labelList->CountItems();
	for (unsigned int i=0; i<n; i++)
	{
		smallLabel *lab = (smallLabel*)labelList->ItemAt(i);
		archive->AddString(hpLn,lab->label.String());
		archive->AddInt32(hpLf,lab->file);
	}
	
	return B_OK;
}

void HTMLProject::SetHome(const BDirectory &dir)
{
	Clean();
	filesDir = dir;
}

unsigned int HTMLProject::AddFile(const char *fname,int guess)
{
	HTMLFile *newFile = new HTMLFile(this,fname);
	int i;
	
	if ( (i=FindFile(newFile,guess)) < 0)
	{
		fileList->AddItem(newFile);
		return fileList->IndexOf(newFile);
	}
	else
	{
		delete newFile;
		return (unsigned int)i;
	}
}

unsigned int HTMLProject::AddLabel(const HTMLLabel *label)
{
	int i;
	static unsigned int lastFile=0;
	
	if ( (i=FindLabel(label)) <0 )
	{
// construction du small label
		smallLabel *newLabel = new smallLabel;
		lastFile = AddFile(label->fname.String(),lastFile);
		
		newLabel->file = lastFile;
		newLabel->label = label->lname;

/*// on cherche le bon endroit pour insérer le small label		
		int p = labelList->CountItems();
		if (p>1)
		{
			do
				p--;
			while ( (p>=1) && ((smallLabel*)labelList->ItemAt(p))->file > lastFile);
		}
			// p=0 est un lien spécial: celui de la page d'intro
		
		labelList->AddItem(newLabel,p);
		return (p);
*/
		labelList->AddItem(newLabel);
		return(labelList->CountItems()-1);
	}
	else
		return (unsigned int)i;
}

void HTMLProject::Update(HappyLink*)
{
	Clean();
}

const unsigned int *HTMLProject::GiveConfig() const
{
	return NULL;
}

void HTMLProject::About() const
{
}

bool HTMLProject::FindHome()
{
	return true;
}

const char** HTMLProject::GiveSupportedAddOns() const
{
	return NULL;
}

void HTMLProject::Clean()
{
	unsigned int n = fileList->CountItems();
	for (unsigned int i=0; i<n; i++)
		delete (HTMLFile*)fileList->RemoveItem((int32)0);
	
	n = labelList->CountItems();
	for (unsigned int i=0; i<n; i++)
		delete (smallLabel*)labelList->RemoveItem((int32)0);
}

int HTMLProject::FindLabel(const HTMLLabel *label)
{
	HTMLFile *file = label->GetFile();
	int fn = FindFile(file);
	delete file;
	
	if (fn<0)
		return -1;
	
	unsigned int n = labelList->CountItems();
	if (n==0)
		return -1;
		
	for (unsigned int i=0; i<n; i++)
	{
		smallLabel *lab = (smallLabel*)labelList->ItemAt(i);
		if (lab->file == (unsigned int)fn)
		{
			if (lab->label == label->lname)
				return i;
		}
		else
			if (lab->file > (unsigned int)fn)
				return -1;	
	}
	
	return -1;
}

int HTMLProject::FindFile(const HTMLFile *file, int guess)
{
	unsigned int n = fileList->CountItems();

	if ((guess >= 0) && ((unsigned int)guess<n) && (*file == *(HTMLFile*)fileList->ItemAt(guess)))
		return guess;
		
	if (n==0)
		return -1;
	
	for (unsigned int i=0; i<n; i++)
		if(*file == *(HTMLFile*)fileList->ItemAt(i))
			return i;
	
	return -1;
}

BString &HTMLProject::GiveName(unsigned int labelNum)
{
	BString *result = new BString("");
	
	smallLabel *theLabel = (smallLabel*)labelList->ItemAt(labelNum);
	if (theLabel == NULL)
		return *result;
	
	HTMLFile *theFile = (HTMLFile*)fileList->ItemAt(theLabel->file);
	if (theFile == NULL)
		return *result;
	
	BPath *path = GetPath(theFile->name.String());
	*result = path->Path();
	delete path;
	
	if (theLabel->label != "")
	{
		*result += '#';
		*result += theLabel->label.String();
	}
	return *result;
}

void HTMLProject::Print(const char *line1,const char *line2)
{
	BMessage print('PRNT');
	if (line1 != NULL)
		print.AddString("line1",line1);
	
	if (line2 != NULL)
		print.AddString("line2",line2);
	
	infoMess.SendMessage(&print);
}

void HTMLProject::NumPrint(unsigned int n)
{
	BMessage numPrint('NPRT');
	numPrint.AddInt32("max",n);
	
	infoMess.SendMessage(&numPrint);
}

const char *HTMLProject::Translate(const char *str) const
{
#ifndef DK_LIB
	return (dynamic_cast<BeHappy*>(be_app))->AddOnTranslate(this,str);
#else
	return str;
#endif
}
	