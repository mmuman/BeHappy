/*
 * BeHappy
 *
 * Par Sylvain Tertois
 *
 * Thread de gestion de fenêtres
 * Sert entres autres à bien gérer la fermeture de BeHappy tranquillement sans qu'il 
 * y ait de problèmes
 *
 */

#include "WindowMonitor.h"
#include "Application.h"
#include <MessageRunner.h>
#include <Window.h>


WindowMonitor::WindowMonitor()
	: BLooper("Window Monitor")
{
	Run();
	m_NumWindows = 0;
	m_QuitTimeOut = NULL;
}

WindowMonitor::~WindowMonitor()
{
	delete m_QuitTimeOut;
}

void WindowMonitor::MessageReceived( BMessage *message)
{
	switch(message->what)
	{
		case 'Open':
			m_NumWindows++;
			break;
		
		case 'Clos':
			if (--m_NumWindows == 0)
			{
				be_app->PostMessage(B_QUIT_REQUESTED);
				PostMessage(B_QUIT_REQUESTED);
			}
			break;

		case 'TOut':
			// il y a eu un time out en fermant les fenêtre... tant pis, on quitte
			be_app->PostMessage(B_QUIT_REQUESTED);
			PostMessage(B_QUIT_REQUESTED);
			break;
			
		default:
			BLooper::MessageReceived(message);
	}
}

void WindowMonitor::BroadcastMessageToWindows( BMessage *message)
{
	BWindow *win;
	for (unsigned int i=0; (win=be_app->WindowAt(i)) != NULL; i++)
	{
		win->PostMessage(message);
	}
}

void WindowMonitor::BroadcastMessageToWindows(int32 command)
{
	BWindow *win;
	for (unsigned int i=0; (win=be_app->WindowAt(i)) != NULL; i++)
	{
		win->PostMessage(command);
	}
}

void WindowMonitor::CloseAll()
{
	// on ne quitte pas tant qu'il y a des fenêtres encore ouvertes
	if (m_NumWindows != 0)
	{
		// on regarde si le messagerunner pour le timeout est déjà mis en place
		if (m_QuitTimeOut == NULL)
		{
			// non... on va le faire
			BMessage l_Message('TOut');
			m_QuitTimeOut = new BMessageRunner(BMessenger(this),&l_Message,20000000,1);
				// 20 secondes de timeout
			
			// on va demander à toutes les fenêtres de se fermer
			for (int i=0; i<be_app->CountWindows(); i++)
			{
				BWindow *l_Win = be_app->WindowAt(i);
				if (l_Win != NULL)
					l_Win->PostMessage(B_QUIT_REQUESTED);
			}
		}
	}	
	else
	// il y en a plus... on ferme
		PostMessage(B_QUIT_REQUESTED);
}
