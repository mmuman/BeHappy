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

// Application

#ifndef BEHAPPY_H
#define BEHAPPY_H

#include <Application.h>

class LanguageHandler;
class HTMLProject;
class BMessageRunner;
class BDirectory;
class BPath;
class WindowMonitor;

class BeHappy : public BApplication
{
public:
	BeHappy();
	~BeHappy();
	
	void MessageReceived(BMessage*);
	void AboutRequested();
	bool QuitRequested();
	
	status_t GetSupportedSuites(BMessage *message);
	BHandler *ResolveSpecifier(BMessage*,int32,BMessage*,int32,const char*);
	void ReadyToRun();

	BDirectory *prefsDir;

	static bool CheckAddOn(const char *path,bool alert,BString *pName=NULL);
	const BPath *GetAddOnPath(const char *name);
	
	void SearchAddOns();
	
	void InitLocale();	// initialise la localisation

	// fonction appelée par un objet HTMLProject quand il veut une traduction
	const char *AddOnTranslate(const HTMLProject *, const char *) const;
	
	// fonction appelée par une fenêtre quand elle ferme
	void WindowClosed();
	
	BList addOnsPaths;
	BList addOnsNames;
	
private:
	bool addOnsSearched;
	bool bookOpened;
	
	LanguageHandler *m_LanguageHandler;	// truc pour gérer automatiquement
			// les changements du fichier de localisation
	WindowMonitor *m_Monitor;
};

class Preferences;
extern Preferences *prefs;

class Eraser;
extern Eraser *eraser;

class Locale;
extern Locale *g_Locale;

// messages inter-threads
// changement de langue
#define GM_LANGUAGE 'Lang'
#define GM_L_NAME "Language Name"

// refaire les menus add-ons
#define BH_ADDONS_RESCANNED 'AdRs'

#endif //BEHAPPY_H
