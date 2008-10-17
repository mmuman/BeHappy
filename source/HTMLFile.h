/*
* Projet HTML
*
* lecture dans un fichier HTML
*
*/

#ifndef HTMLFILE_H
#define HTMLFILE_H

class HTMLProject;
#include "String.h"
class HTMLBeacon;
#include <List.h>

class HTMLFile
{
public:
	HTMLFile(HTMLProject *proj, const char *fname);
	~HTMLFile();
	
	bool operator==(const HTMLFile&) const;
	inline bool operator!=(const HTMLFile &f2) const { return !operator==(f2); }
	
	void GetLists(unsigned int beacon1, unsigned int beacon2,BList *links,BList *labels);
	void GetLists(HTMLBeacon *beacon1, HTMLBeacon *beacon2,BList *links,BList *labels);
		// les éléments des listes ne doivent PAS être détruits	

	inline void AddBeacon(HTMLBeacon *b) { beacon->AddItem(b); }
		// HTMLFile garde le beacon et se chargera de sa destruction

	status_t Search();
	void Dump();
		
private:
	bool searchDone;
	
	void FoundLabel(BString &name,BString &text,unsigned int bnum);
	void FoundLink(BString &name,BString &text,unsigned int bnum);
	void FoundAToken(char *data,int p1,int p2,int p3,int p4,unsigned int bnum);
	void FoundBeacon(char *data,int p1,int p2,HTMLBeacon *whereToPut);
	void ChangeString(BString&) const;	// enlève les "<...>" et change les "%20" en " "
	

	friend class HTMLProject;
	
	HTMLProject *proj;
	BString name;
	
	BList *beacon;
	BList *links;
	BList *labels;
};

class HTMLBeacon
{
public:
	HTMLBeacon(const char *stringToFind);
	HTMLBeacon(const char *string1, const char *string2);
	
	inline bool Found() { return found; }
	inline const char *FoundString() { return foundStr.String(); }

	friend class HTMLFile;
	
private:
	BString str1,str2;
	bool found;
	BString foundStr;
};

class HTMLLabel
{
public:
	HTMLLabel(HTMLProject *proj,const char *fname, const char *lname, const char *ltext, unsigned int beacon=0);
	HTMLLabel(const HTMLLabel*);
	~HTMLLabel();

	inline HTMLFile *GetFile() const { return new HTMLFile(proj,fname.String()); }
			// Fichier à détruire après utilisation
	inline const char *LabelName() const { return lname.String(); }
	inline const char *LabelText() const { return ltext.String(); }
	inline const char *FileName() const { return fname.String(); }
			// Noms à ne pas détruire après utilisation
			
	friend class HTMLFile;
	friend class HTMLProject;
	
	bool operator==(const HTMLLabel&) const;
	inline bool operator!=(const HTMLLabel &l) const { return !operator==(l); }

private:
	HTMLProject *proj;
	BString fname,lname,ltext;
	unsigned int beacon;
};
	
#endif //HTMLFILE_H
	