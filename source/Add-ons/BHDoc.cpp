/*
* BeHappy Documentation Add-on 
*
* (c) 1999 Sylvain Tertois
*
*/

#include "BHDoc.h"
#include "HappyLink.h"
#include "HTMLFile.h"
#include "BHAdd-ons.h"
#include <Alert.h>
#include <Roster.h>

// traduction
#ifdef LANG_FRANCAIS
const char *g_l_Name = "Documentation de BeHappy";
const char *g_l_Summary = "Sommaire";
const char *g_l_Parsing = "Lecture:";
const char *g_l_Classes = "Classes";
const char *g_l_Topics = "Sujets";
const char *g_l_About = "Documentation de BeHappy\nPar Sylvain Tertois";
const char *g_l_OK = "OK";
#else
const char *g_l_Name = "BeHappy Documentation";
const char *g_l_Summary = "Summary";
const char *g_l_Parsing = "Parsing:";
const char *g_l_Classes = "Classes";
const char *g_l_Topics = "Topics";
const char *g_l_About = "BeHappy Documentation\nBy Sylvain Tertois";
const char *g_l_OK = "OK";
#endif

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
const char *projectName=g_l_Name;

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
	static unsigned int config[] = { 0,BW_SECONDLIST,BW_SECONDLIST };
	return(config);
}

// Put here the code to find the documentation path
// if you return false, a window will be opened asking for the path
bool AddOn::FindHome()
{
	// first try to find the "BeHappyDK" folder
	BQuery myQuery;
	BVolumeRoster roster;
	
	/// get the boot volume
	BVolume vol;
	roster.GetBootVolume(&vol);
	myQuery.SetVolume(&vol);
	
	/// sets and launch the Query
	myQuery.SetPredicate("name=BeHappyDK");
	myQuery.Fetch();
	
	/// and look what we've found
	BEntry myEntry;
	while(myQuery.GetNextEntry(&myEntry) == B_OK)
	{
		//// if it isn't a directory, give up
		if (!myEntry.IsDirectory())
			continue;
		
		//// try to build a BDirectory object to see if the Doc folder is here
		BDirectory myDir(&myEntry);
		
		if (myDir.SetTo(&myDir,"Doc") == B_OK)
		{
			//// if the init is OK, we've found it!
			SetHome(myDir);
			return true;
		}
	}	
	
	// it seems that the BeHappyDK folder isn't here...
	// look for BeHappy's folder
	entry_ref beHappyRef;
	BRoster appRoster;
	if (appRoster.FindApp("application/x.vnd-STertois.BeHappy",&beHappyRef) == B_OK)
	{
		/// find BeHappy's directory
		BDirectory happyDir;
		BEntry beHappyApp(&beHappyRef);
		beHappyApp.GetParent(&happyDir);
		
		/// and the documentation directory
		if (happyDir.SetTo(&happyDir,"Doc") == B_OK)
		{
			//// if the init is OK, we've found it!
			SetHome(happyDir);
			return true;
		}
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
	
		/// and here we go
		BDirectory homeDir(homePath->Path());
		NumPrint(CountHTMLFiles(homeDir));
	}
	
	// create the happyList
	happyList = new HappyLink(this,projectName,mainFile);
	
	// create the first index
	HappyLink *index = new HappyLink(this,g_l_Summary,mainFile);
	
	// maybe the two other indexes... we'll see later!!
	classes = topics = NULL;
	
	// add the summary to the happy list:
	happyList->AddChild(index);	// this can be done before or after you fill in the index

	// and add the files!!
	HTMLFile *theFile = mainFile->GetFile();
	NewFile(theFile,index);
	delete theFile;
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
		BString info = g_l_Parsing;
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
			classes = new HappyLink(this,g_l_Classes,mainFile);
			topics = new HappyLink(this,g_l_Topics,mainFile);
			
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
	BAlert *info = new BAlert("BeHappy:BeHappy Doc",g_l_About,g_l_OK);
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