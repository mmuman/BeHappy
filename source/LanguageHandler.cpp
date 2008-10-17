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
 
 #include "LanguageHandler.h"
 #include <NodeMonitor.h>
 #include <Entry.h>
 #include "BeHappy.h"
 
 LanguageHandler::LanguageHandler()
 : BHandler("LanguageHandler")
 {
 }
 
 LanguageHandler::~LanguageHandler()
 {
 	stop_watching(this);
 }
 
 void LanguageHandler::InstallNodeMonitor(const BEntry *file)
 {
 	node_ref l_MyNodeRef;
 	file->GetNodeRef(&l_MyNodeRef);
 	
 	watch_node(&l_MyNodeRef,B_WATCH_STAT,this);
 }
 
 void LanguageHandler::MessageReceived(BMessage *message)
 {
 	switch(message->what)
 	{
 		case B_NODE_MONITOR:
 			be_app->PostMessage(GM_LANGUAGE);
 		break;
 		
 		default:
 			BHandler::MessageReceived(message);
 		break;
 	}
 }