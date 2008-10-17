/*
* BeHappy
* 
* Projet HTML
*
*/

#ifndef HTMLPROJ_H
#define HTMLPROJ_H

#include <StorageKit.h>
class HTMLLabel;
class HTMLFile;
class HappyLink;

class HTMLProject : public BArchivable
{
public:
	HTMLProject();
	HTMLProject(BMessage *archive);
	~HTMLProject();
	
	inline bool InitOK() { return initOK; }
	
	static BArchivable *Instantiate(BMessage*);
	status_t Archive(BMessage*,bool deep = true);

	void SetHome(const BDirectory &dir);
		
	unsigned int AddFile(const char *fname, int guess=-1);
	unsigned int AddLabel(const HTMLLabel *label);
		// pour ces deux méthodes, l'appelant garde la responsabilité
		// des objets (HTMLProject en fait une copie)

	inline HTMLFile *GetFile(unsigned int index) { return (HTMLFile*)fileList->ItemAt(index); }

	inline BPath *GetPath(const char *fname) { return new BPath(&filesDir,fname); }
		// le BPath est à effacer après utilisation
		
	virtual bool FindHome();
	virtual void Update(HappyLink*);
	virtual const unsigned int *GiveConfig() const;
	virtual void About() const;
	
		// pas encore utilisé
	virtual const char** GiveSupportedAddOns() const;
	
	HappyLink *happyList;
	
	BString &GiveName(unsigned int);
		// rend le nom complet d'un lien (fichier#lien)

	inline void SetInfoMessenger(BMessenger &m) {infoMess = m;}
	void Print(const char *line1,const char *line2 = NULL);
	void NumPrint(unsigned int n);
	
	const char *Translate(const char *str) const;
		
protected:
	void Clean();

private:
	int FindLabel(const HTMLLabel *label);
		// -1 si lien non trouvé
	int FindFile(const HTMLFile *file, int guess=-1);
		// -1 si fichier non trouvé
			
	bool initOK;
	
	BList *fileList;
	BList *labelList;	// c'est une liste de smallLabel, et pas HTMLLabel!!
	
	BDirectory filesDir;
	
	BMessenger infoMess;
};	

#endif