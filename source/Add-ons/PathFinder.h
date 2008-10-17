/*
 * BeHappy
 * ©1999-2001 Sylvain Tertois
 *
 * Add-on pour le BeBook
 *
 * Looper qui cherche des BeBooks
 *
 */

#ifndef PATHFINDER_H
#define PATHFINDER_H

#include <Looper.h>
#include <Messenger.h>
#include <Query.h>

class PathFinder : public BLooper
{
public:
	PathFinder(BMessenger);

	void MessageReceived(BMessage*);

	void AddPath(entry_ref*);
		// envoie un message donnant un chemin de bebook
		// l'entry_ref pointe vers un sous-dossier du bebook
		// (ici le dossier "The Interface Kit")
private:
	BMessenger m_Messenger;
		// messager qui reçoit les messages concernant les chemins
	BQuery m_Query;
};

// messages envoyés
#define PF_ADD_PATH 'AdPa'
#define PF_REMOVE_PATH 'RmPa'

// contenu des messages
#define PF_AR_PATH "path"

#endif // PATHFINDER_H