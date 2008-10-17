/*
 * BeHappy
 * ©1999-2001 Sylvain Tertois
 *
 * Add-on pour le BeBook
 *
 * Looper qui cherche des BeBooks
 *
 */

#include "PathFinder.h"
#include <Entry.h>
#include <NodeMonitor.h>
#include <Path.h>
#include <Volume.h>
#include <VolumeRoster.h>
#include <Directory.h>


PathFinder::PathFinder(BMessenger mes)
: m_Messenger(mes)
{
	PostMessage('Strt');
	
	Run();
}

void PathFinder::MessageReceived(BMessage *message)
{
	switch(message->what)
	{
		case 'Strt':
		{
			// c'est parti!
			// on commence par établir la query
			BVolume l_Volume;
			BVolumeRoster l_VolumeRoster;
			l_VolumeRoster.GetBootVolume(&l_Volume);
			m_Query.SetVolume(&l_Volume);
			
			m_Query.PushAttr("name");
			m_Query.PushString("The Interface Kit",true);
			m_Query.PushOp(B_EQ);
			m_Query.SetTarget(BMessenger(this));
			m_Query.Fetch();
			
			// on boucle dans les dossiers qu'on trouve
			entry_ref l_ref;
			while( m_Query.GetNextRef(&l_ref) == B_OK)
			{
				AddPath(&l_ref);
			}
		}
		break;
		
		case B_QUERY_UPDATE:
		{
			// bon... c'est quel update?
			int32 l_Opcode;
			message->FindInt32("opcode",&l_Opcode);
			if (l_Opcode == B_ENTRY_CREATED)
			{
				// il y a un nouveau chemin
				entry_ref l_ref;
			
				// extraction du ref (source BeBook)
				const char *l_name;
				message->FindInt32("device", &l_ref.device);
				message->FindInt64("directory", &l_ref.directory);
				message->FindString("name", &l_name);
				l_ref.set_name(l_name);
			
				// envoi du message
				AddPath(&l_ref);
			}
		}
		break;
		
		default:
			BLooper::MessageReceived(message);
		break;
	}
}

void PathFinder::AddPath(entry_ref *ref)
{
	BEntry l_Entry(ref);
	BPath l_FolderPath;
	
	// on vérifie deux ou trois trucs:
	// est-ce un dossier?
	BDirectory l_Dir(&l_Entry);
	if (l_Dir.InitCheck() != B_OK)
		return;	// non
	// y-a-t'il le fichier index.html
	if (!l_Dir.Contains("index.html"))
		return;	// non
		
	// on récupère le parent, c'est celui-là qu'on veut
	if ((l_Entry.GetParent(&l_Entry) == B_OK) && (l_Entry.GetPath(&l_FolderPath) == B_OK))
	{
		// construction du message à envoyer
		BMessage l_Message(PF_ADD_PATH);
		l_Message.AddString(PF_AR_PATH,l_FolderPath.Path());
			
		m_Messenger.SendMessage(&l_Message);
	}
}