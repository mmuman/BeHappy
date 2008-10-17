/*
***********************
*                     *
*       BeHappy       *
*                     *
*---------------------*
*                     *
* par Sylvain Tertois *
*                     *
***********************
*/

// Fenêtre principale

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <Window.h>
class NetView;
class BFilePanel;
class HTMLProject;
class BrowseView;
class Mover;
class AddOnPrefFile;
class BMenuBar;

#include <String.h>
#include <Path.h>

#include "Locale.h"

class Tasks;
class MainWindow : public BWindow
{
public:
	MainWindow(const char *openThis = NULL);
	~MainWindow();

	void BuildMenu();
		
	void MessageReceived(BMessage*);
	status_t GetSupportedSuites(BMessage *message);
	BHandler *ResolveSpecifier(BMessage*,int32,BMessage*,int32,const char*);
	
	void NewProject(const char*);
	void GoTo(unsigned int link);
	
	inline const char *AddOnTranslate(const char *string) const
		// traduit avec le fichier de localisation de l'add-on
	{ return m_AddOnLocale->Translate(string); }
	
	// pour l'objet BeHappy
	inline bool HTMLProjectIs(const HTMLProject *proj) const
	{ return proj == project; }
	
private:
	void UpdateProject();
	void UpdateBrowse();

private:
	void LoadMoversPosition();
	void SaveMoversPosition();
	void SaveBookName();
	void UpdateBookMenu();
	
	NetView *theNetView;
	BMenuField *chooseList;
	BrowseView *browse;
	BView *leftView;
	Mover *verticalMover;
	BMenu *addOnMenu,*bookMenu;
	BMenuBar *m_WindowMenuBar;
	BMenuItem *updateItem;
	HTMLProject *project;
	BString projectName;
	AddOnPrefFile *projectPrefs;
	Tasks *tasks;
	
	image_id currentProject;
	BString currentProjectPath;
	const char *currentOpenedIndex;
	
	bool m_AddOnLoaded;
	Locale *m_AddOnLocale;	// localisation de l'add-on	
	friend class Tasks;
};

typedef HTMLProject* (*instantiate_project)(BMessage *archive=NULL);

#endif