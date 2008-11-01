/*
* Man pages Add-on 
*
* (c) 2003 François Revol
*
*/

#include "Add-On.h"
#include "HappyLink.h"
#include "HTMLFile.h"
#include "BHAdd-ons.h"
#include <Alert.h>
#include <Roster.h>

// don't touch these!
// these two variables are used by BeHappy to be sure it's compatible with the add-on
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
const char *projectName="Man pages";
const char *htmlPath="/boot/beos/documentation/Shell Tools";

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
	static unsigned int config[] = { 0,BW_SECONDLIST,BW_SEARCHTXTV|BW_CONTAINS };
	return(config);
}

// Put here the code to find the documentation path
// if you return false, a window will be opened asking for the path
bool AddOn::FindHome()
{
	//// try to build a BDirectory object to see if the Doc folder is here
	BDirectory myDir(htmlPath);
	
	if (myDir.InitCheck() == B_OK)
	{
		//// if the init is OK, we've found it!
		SetHome(myDir);
		return true;
	}

	// not found... ask user
	SetHome(BDirectory("/boot/home"));
	return false;
}

// The biggest part: put here your code to build the indexes
// don't use the HappyLink* (yet)
void AddOn::Update(HappyLink*)
{
	// first let's clean the old indexes if they're there
	Clean();
	
	// now let's go!! put your code here
	HTMLLabel *mainFile = new HTMLLabel(this,"index.html","","Index");
	
	// count the number of HTML files in the home directory, to tell the
	// infowindow how many Print() calls it will get
	{
		/// first get the path of the home directory
		BPath *homePath = GetPath("");
		int htmlFileCount = 0;
	
		/// and here we go
		BDirectory homeDir(homePath->Path());
		BEntry ent;
		BDirectory mdir;
		while (homeDir.GetNextEntry(&ent) == B_OK) {
			mdir.SetTo(&ent);
			htmlFileCount += CountHTMLFiles(mdir);
		}
		NumPrint(htmlFileCount);
	}
	
	// create the happyList
	happyList = new HappyLink(this,projectName,mainFile);
	
	// create the first index
	HappyLink *index = new HappyLink(this,"By Section",mainFile);
	
	// create the second index
	HappyLink *index2 = new HappyLink(this,"Splited List",mainFile);
	
	// create the third index
	HappyLink *index3 = new HappyLink(this,"Search",mainFile);
	
	// maybe the two other indexes... we'll see later!!
	classes = topics = NULL;
	
	// add the summary to the happy list:
	happyList->AddChild(index);	// this can be done before or after you fill in the index
	happyList->AddChild(index2);	// this can be done before or after you fill in the index
	happyList->AddChild(index3);	// this can be done before or after you fill in the index

	// and add the files!!
	{
		/// first get the path of the home directory
		BPath *homePath = GetPath("");
		int htmlFileCount = 0;
	
		/// and here we go
		BDirectory homeDir(homePath->Path());
		BEntry ent;
		BDirectory mdir;
		//HTMLFile *theFile = mainFile->GetFile();

		while (homeDir.GetNextEntry(&ent) == B_OK) {
			char fname[B_FILE_NAME_LENGTH];
			char name[B_FILE_NAME_LENGTH];
			mdir.SetTo(&ent);
			if (mdir.InitCheck() != B_OK)
				continue; /* not a folder */
			ent.GetName(fname);
			if (strncmp(fname, "man", 3))
				continue; /* not man? */
			HappyLink *section = new HappyLink(this, SectionName(fname), mainFile);
			HappyLink *section2 = new HappyLink(this, SectionName(fname), mainFile);
			index->AddChild(section);
			index2->AddChild(section2);
			
			while (mdir.GetNextEntry(&ent) == B_OK) {
				BPath leafPath;
				ent.GetPath(&leafPath);
				ent.GetName(name);
				BString nameStr(name);
				nameStr.Truncate(nameStr.FindLast('.')); // .html
				nameStr.Truncate(nameStr.FindLast('.')); // .xyz
				
				// Update the information in the info window
				BString info = "Parsing:";
				info << nameStr.String();
				Print(NULL,info.String());
				
				// Reuse
				info.SetTo(fname);
				info.Append("/");
				info.Append(name);
				HTMLLabel *pageFile = new HTMLLabel(this,info.String(),"",nameStr.String());
				HappyLink *mpage = new HappyLink(this, nameStr.String(), pageFile);
				section->AddChild(mpage);
				HappyLink *mpage2 = new HappyLink(this, nameStr.String(), pageFile);
				section2->AddChild(mpage2);
				HappyLink *mpage3 = new HappyLink(this, nameStr.String(), pageFile);
				index3->AddChild(mpage3);
				//NewFile(theFile,index);
			}
		}
		//delete theFile;
	}
}

// Add all the labels and links of the given file into the given HappyLink
void AddOn::NewFile(HTMLFile *file,HappyLink *list)
{
	// Add the beacons... 
	/// All the html files have a <!DK> tag, to say what type of file it is
	/// I add a beacon to get this information
	HTMLBeacon *beaconDK = new HTMLBeacon("<!DK><!","><!DK>");
	file->AddBeacon(beaconDK);
	
	/// only the links between <!start> and <!stop> will be useful
	file->AddBeacon(new HTMLBeacon("<!start"));
	file->AddBeacon(new HTMLBeacon("<!stop"));
	
	// Parse the file
	file->Search();
	
	if (!beaconDK->Found())
	// the DK tags aren't there... too bad!
		return;
	
	// get the labels and links found in the file
	BList labels,links;
	
	/// first the labels.. we want all of them
	file->GetLists(0,4,NULL,&labels);
	
	/// and the links... I only want those between <!start> and <!stop>
	file->GetLists(2,3,&links,NULL);
	
	// Add all the labels to the summary
	int n = labels.CountItems();
	for (int i=0; i<n; i++)
	{
		HTMLLabel *theLabel = (HTMLLabel*)labels.ItemAt(i);
		list->AddChild(new HappyLink(this,theLabel->LabelText(),theLabel));
	}
	
	// And the links
	n = links.CountItems();
	for (int i=0; i<n; i++)
	{
		HTMLLabel *theLink = (HTMLLabel*)links.ItemAt(i);

		// Update the information in the info window
		BString info = "Parsing:";
		info << theLink->FileName();
		Print(NULL,info.String());

		// Add a link in the summary
		HappyLink *child = new HappyLink(this,theLink->LabelText(),theLink);
		list->AddChild(child);
		
		// parse the new file, and add its links and labels to the summary, 
		// as children of child.
		HTMLFile *newFile = theLink->GetFile();
		NewFile(newFile,child);
		delete newFile;
	}
	
	// find if the file is a development kit topic, or a class
	BString dkType = beaconDK->FoundString();
	
	if (dkType != "no")
	{
		// This is a DK file
		// is it the first one?
		if (classes == NULL)
		{
			// yes, create the other indexes
			HTMLLabel *mainFile = new HTMLLabel(this,"index.html","","Index");
			classes = new HappyLink(this,"Classes",mainFile);
			topics = new HappyLink(this,"Topics",mainFile);
			
			// add the two indexes to the happyList
			happyList->AddChild(classes);
			happyList->AddChild(topics);
		}
		
		// create a copy of the HappyLink
		HappyLink *copy = new HappyLink(*list);
		
		// is it a class or a topic?
		if (dkType == "class")
			// class
			classes->AddChild(copy);
		else
			// topic
			topics->AddChild(copy);
			
	}
}
	

// About box
void AddOn::About() const
{
	BAlert *info = new BAlert("BeHappy:Man","Man pages\nby François Revol","OK");
	info->Go();
}

// Count the HTML files in a directory
unsigned int AddOn::CountHTMLFiles(BDirectory &dir)
{
	BEntry file;
	unsigned int count = 0;
	
	// loop in all the files in the directory
	while (dir.GetNextEntry(&file,true) == B_OK)
	{
		if (file.IsDirectory())
		{
			// if it a directory, we'll have to look for html files in it
			BDirectory newDir(&file);
			count += CountHTMLFiles(newDir);
		}
		else
		{
			// find the MIME type
			BNode myNode(&file);
			BNodeInfo myNodeInfo(&myNode);
			
			// is it an html file?
			char mime[256];
			myNodeInfo.GetType(mime);
			if (strcmp(mime,"text/html") == 0)
				count++;
		}
	}
	
	return count;
}

const char *AddOn::SectionName(const char *n)
{
	if (!strcmp(n, "man1"))
		return "1 - General Commands";
	if (!strcmp(n, "man2"))
		return "2 - System Calls";
	if (!strcmp(n, "man3"))
		return "3 - Subroutines";
	if (!strcmp(n, "man4"))
		return "4 - Special Files";
	if (!strcmp(n, "man5"))
		return "5 - File Formats";
	if (!strcmp(n, "man6"))
		return "6 - Games";
	if (!strcmp(n, "man7"))
		return "7 - Macros and Conventions";
	if (!strcmp(n, "man8"))
		return "8 - Maintenance Commands";
	if (!strcmp(n, "manl"))
		return "l - SQL Commands";
	if (!strcmp(n, "mann"))
		return "n - Tcl-Tk ?";
	return n;
}
