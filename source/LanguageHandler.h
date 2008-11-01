/*
 * BeHappy
 *
 * par Sylvain Tertois
 *
 *
 * LanguageHandler
 *
 * Ce Handler sert à surveiller le fichier de localisation
 * à chaque fois que ce fichier est modifié, toutes les fenêtres
 * sont refaites
 *
 */
 
 #ifndef LANGUAGEHANDLER_H
 #define LANGUAGEHANDLER_H
 
 #include <Handler.h>
class BEntry;
 
 class LanguageHandler : public BHandler
 {
 public:
 	LanguageHandler();
 	~LanguageHandler();
 	
 	void InstallNodeMonitor(const BEntry *file);
 	
 	virtual void MessageReceived(BMessage*);
 };
 #endif
