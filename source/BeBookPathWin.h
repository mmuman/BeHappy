/*
 * BeHappy
 * ©1999-2001 Sylvain Tertois
 *
 * Add-on pour le BeBook
 *
 * Fenêtre de choix de chemin pour le BeBook
 *
 */

#ifndef BEBOOKPATHWIN_H
#define BEBOOKPATHWIN_H

#include <Window.h>
#include <OS.h>
#include <Locker.h>
#include <String.h>

class HTMLProject;
class PathFinder;

class BeBookPathWin : public BWindow
{
public:
	BeBookPathWin(HTMLProject *add_on);
	~BeBookPathWin();
	
	void MessageReceived(BMessage*);
	bool QuitRequested();
	
	char *ChoosePath();

private:
	sem_id m_WaitForEndSema;
		// sémaphore mis par la fenêtre qui appelle celle-ci pour attendre sa réponse

	BView *m_MainView;
	int m_NumOfViews;	// nombre d'enfants de la m_MainView, sans
		// compter les boutons radios
	
	float m_Y;	// emplacement du prochain contrôle à mettre dans la fenêtre

	PathFinder *m_PathFinder;	// looper qui va chercher les BeBooks installés
	
	BString m_CurrentPath;
		// chemin actuellement sélectionné
		
	BLocker m_DeleteLocker;
		// vérrou appelé par QuitRequested pour attendre que ChoosePath() soit
		// bien terminé avant de détruire l'objet
};

#endif
	