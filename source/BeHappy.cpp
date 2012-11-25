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

#include "BeHappy.h"
#include "MainWindow.h"
#include "Preferences.h"
#include <Alert.h>
#include <Roster.h>
#include <StorageKit.h>
#include <PropertyInfo.h>
#include "LanguageHandler.h"
#include "Version.h"
#include "Eraser.h"
#include "Locale.h"
#include "WindowMonitor.h"

// macro utilisée pour les traductions
#define T(x) g_Locale->Translate(x)


Preferences *prefs;
Eraser *eraser;
Locale *g_Locale;	// objet utilisé pour la localisation

int main()
{
	BeHappy myApp;
	
	myApp.Run();
}

BeHappy::BeHappy()
	: BApplication("application/x.vnd-STertois.BeHappy")
{
	addOnsSearched = false;
	
// récupération du dossier de préférences
	BPath prefsPath;
	find_directory(B_USER_SETTINGS_DIRECTORY,&prefsPath);
#ifdef _DEBUG
	prefsPath.Append("BeHappy_");
#else
	prefsPath.Append("BeHappy");
#endif //_DEBUG
	BEntry prefsDirEntry(prefsPath.Path());
	prefsDir = new BDirectory(prefsPath.Path());
	
	// si ce nom est celui d'un fichier et pas celui d'un dossier, il faut
	// effacer le fichier
	if ((prefsDirEntry.InitCheck() == B_OK) &&
		(prefsDir->InitCheck() != B_OK))
		prefsDirEntry.Remove();
	
	// si le dossier n'existe pas, on le crée
	if (prefsDir->InitCheck() != B_OK)
	{
		create_directory(prefsPath.Path(),0777);
		prefsDir->SetTo(prefsPath.Path());
	}
	prefsPath.Append("Preferences");
			
	prefs = new Preferences(prefsPath.Path());
	InitLocale();
	SearchAddOns();
	
	eraser = new Eraser;
	
	bookOpened = false;
	
	// création du monitor
	m_Monitor = new WindowMonitor;
}

BeHappy::~BeHappy()
{
	stop_watching(be_app_messenger);
	delete prefs;
}

void BeHappy::MessageReceived(BMessage *message)
{
	switch(message->what)
	{
		case B_SILENT_RELAUNCH:
		case 'Open':
		{
			const char *addOnName;
			if (message->FindString("Name",&addOnName) == B_OK)
			{
				bookOpened = true;
				// on nous demande d'ouvrir un livre particulier
				const BPath *addOnPath = GetAddOnPath(addOnName);
				if (addOnPath == NULL)
				{
					// add-on inconu
					message->SendReply(B_MESSAGE_NOT_UNDERSTOOD);
					return;
				}
				else
				{
					BMessage reply(B_REPLY);
					reply.AddMessenger("Messenger",BMessenger(new MainWindow(addOnPath->Path())));
					message->SendReply(&reply);
				}
			}
			else
			{
				// on ouvre simplement une nouvelle fenêtre
				new MainWindow;
			}
			// on signal au moniteur qu'il y a une nouvelle fenêtre
			m_Monitor->PostMessage('Open');
		}
		break;
					
		case 'Hide':
			// on veut que BeHappy n'ouvre pas de fenêtre au démarrage
			// donc on fait comme si un livre a été ouvert, pour éviter que
			// ça se fasse dans ReadyToRun()
			bookOpened = true;
		break;
				
		case B_NODE_MONITOR:
		{
			// il y a eu une modif dans le répertoire add-ons...
			// on refait la recherche
			SearchAddOns();
			
			// et on prévient les autres fenêtres
			m_Monitor->BroadcastMessageToWindows(BH_ADDONS_RESCANNED);
/*			BWindow *win;
			for (unsigned int i=0; (win=WindowAt(i)) != NULL; i++)
			{
				win->PostMessage(BH_ADDONS_RESCANNED);
			}
*/		}
		break;
		
		// Scripting
		case B_COUNT_PROPERTIES:
		{
			int32 index,spWhat;
			BMessage specifier;
			const char *spProp;
			if (message->GetCurrentSpecifier(&index,&specifier,&spWhat,&spProp)==B_OK)
			{
				// on vérifie que c'est bien pour nous
				if (BString(spProp) == "Add-ons")
				{
					// on veut compter le nombre de "add-on", donc ici, c'est un direct specifier
					if (spWhat == B_DIRECT_SPECIFIER)
					{
						BMessage reply(B_REPLY);
						reply.AddInt32("result",addOnsNames.CountItems());
						reply.AddInt32("error",B_OK);
						
						message->SendReply(&reply);
					}
					else
					{
						// le message de scripting est mal foutu
						BMessage reply(B_MESSAGE_NOT_UNDERSTOOD);
						reply.AddInt32("error",B_BAD_SCRIPT_SYNTAX);
						message->SendReply(&reply);
					}
				}
				else
					BApplication::MessageReceived(message);
			}
			else
				BApplication::MessageReceived(message);
		}
		break;

		case B_GET_PROPERTY:
		{
			int32 index,spWhat;
			BMessage specifier;
			const char *spProp;
			if (message->GetCurrentSpecifier(&index,&specifier,&spWhat,&spProp)==B_OK)
			{
				// on vérifie que c'est bien pour nous
				if (BString(spProp) == "Add-on")
				{
					// on veut le nom d'un add-on... on doit avoir un index specifier
					if ((spWhat == B_INDEX_SPECIFIER) && (specifier.FindInt32("index",&index) == B_OK))
					{
						BString *str = (BString*)addOnsNames.ItemAt(index);
						if (str==NULL)
						{
							// l'index donné est trop grand
							BMessage reply(B_MESSAGE_NOT_UNDERSTOOD);
							reply.AddInt32("error",B_BAD_INDEX);
							message->SendReply(&reply);
						}
						else	
						{
							BMessage reply(B_REPLY);
							reply.AddString("result",str->String());
							reply.AddInt32("error",B_OK);
							message->SendReply(&reply);
						}
					}
					else
					{
						// le message de scripting est mal foutu
						BMessage reply(B_MESSAGE_NOT_UNDERSTOOD);
						reply.AddInt32("error",B_BAD_SCRIPT_SYNTAX);
						message->SendReply(&reply);
					}
				}
				else
					BApplication::MessageReceived(message);
			}
			else
				BApplication::MessageReceived(message);
		}
		break;
		
		// changement de langue
		case GM_LANGUAGE:
		{
			const char *l_LanguageName;
			BString *l_LNString = NULL;
			if (message->FindString(GM_L_NAME,&l_LanguageName) == B_OK)
			{
				// stoquage dans les préférences
				prefs->AddString(l_LanguageName,"Language");
			}
			else
			{
				// le message ne contient pas de langue... ça doit venir du LanguageHandler
				// donc on prend la dernière langue utilisée
				l_LNString = prefs->FindString("English","Language");
				l_LanguageName = l_LNString->String();
			}
				
			// utilisation de la nouvelle langue
			g_Locale->ChangeLanguage(l_LanguageName);
				
			// enfin on prévient les fenêtres
			m_Monitor->BroadcastMessageToWindows(message);
/*			BWindow *win;
			for (unsigned int i=0; (win=WindowAt(i)) != NULL; i++)
			{
				win->PostMessage(message);
			}
*/			
			delete l_LNString;	
		}
		break;
		
/*		// timeout au cours de la procédure de fermeture du programme
		case 'TOut':
			numMainWindows = 0;
			PostMessage(B_QUIT_REQUESTED);
		break;
*/									
		default:
			BApplication::MessageReceived(message);
		break;
	}
}

status_t BeHappy::GetSupportedSuites(BMessage *message)
{
	message->AddString("suites","suite/vnd.STertois-BeHappy-application");
	
	static property_info prop_list[] = {
		{"Add-on",{B_GET_PROPERTY,0},{B_INDEX_SPECIFIER,0},"Add-on name"},
		{"Add-ons",{B_COUNT_PROPERTIES,0},{B_DIRECT_SPECIFIER,0},"Add-on count"},
		{0}};
	BPropertyInfo prop_info(prop_list);
	message->AddFlat("messages",&prop_info);
	return(BApplication::GetSupportedSuites(message));
}

BHandler *BeHappy::ResolveSpecifier(BMessage *message,int32 index,BMessage *specifier,int32 what,const char *property)
{
	// les seules propriétés qu'on gère ici ce sont "add-on" et "add-ons"
	BString str(property);
	if ((str == "Add-on") || (str == "Add-ons"))
		return this;
	else
		return(BApplication::ResolveSpecifier(message,index,specifier,what,property));	
}

void BeHappy::ReadyToRun()
{
	if (!bookOpened)
		// on n'a pas demandé d'ouverture de livre au démarrage... 
		// on ouvre une fenêtre de base
		PostMessage('Open');
}

void BeHappy::AboutRequested()
{
	char tmp[100];
	sprintf(tmp,"BeHappy, version %x.%2.2x\nby Sylvain Tertois",bhv_application>>8,bhv_application&0xff);
	BAlert *info = new BAlert("BeHappy!",tmp,"Send email","OK");
	if (info->Go() == 0)
	{
		char *argv[2]= { "app", "mailto:sylvain@fairesuivre.fr" };
	
		be_roster->Launch("text/x-email",2,argv);
	}
}

void BeHappy::SearchAddOns()
{
	// on vide les listes
	{
		BPath *p;
		while ((p=(BPath*)addOnsPaths.RemoveItem((int32)0))!=NULL)
			delete p;
			
		BString *s;
		while ((s=(BString*)addOnsNames.RemoveItem((int32)0))!=NULL)
			delete s;
	}
		
	// d'abord on cherche le dossier
	app_info myInfo;
	be_app->GetAppInfo(&myInfo);
	BEntry appEntry(&(myInfo.ref));
	BDirectory addOnsDir;
	appEntry.GetParent(&addOnsDir);
	
	// parcours de tous les fichiers du dossier
	if (addOnsDir.SetTo(&addOnsDir,"Add-ons")==B_OK)
	{
		BEntry addOn;
		while (addOnsDir.GetNextEntry(&addOn,true) == B_OK)
		{
			BPath *addOnPath = new BPath;
			addOn.GetPath(addOnPath);
			// extraction du type MIME
			{
				BNode myNode(&addOn);
								
				BNodeInfo myNodeInfo(&myNode);
				char mimeType[256];
				myNodeInfo.GetType(mimeType);
				if (!(BMimeType("application/x-vnd.Be-elfexecutable") == mimeType))
					continue;
			}
			
			// on est sûrs que c'est un Add-on
			BString *projName = new BString;
			if (CheckAddOn(addOnPath->Path(),true,projName))
			{
				addOnsPaths.AddItem(addOnPath);
				addOnsNames.AddItem(projName);
			}
			else
			{
				delete addOnPath;
				delete projName;
			}
			
			// si c'est la première fois que SearchAddOns est appelé, on doit activer le node monitor
			if (!addOnsSearched)
			{
				addOnsSearched = true;
				
				node_ref myRef;
				addOnsDir.GetNodeRef(&myRef);
				watch_node(&myRef,B_WATCH_DIRECTORY,be_app_messenger);
			}
		}
	}
	else
	{
		BAlert *myAlert = new BAlert("BeHappy",T("Can't find Add-ons folder"),
			"Quit",NULL,NULL,B_WIDTH_AS_USUAL,B_STOP_ALERT);
		
		myAlert->Go();
		PostMessage(B_QUIT_REQUESTED);
	}
}

bool BeHappy::CheckAddOn(const char *path,bool alert,BString *pName)
{
	image_id addOnImage = load_add_on(path);
	void *instFunc;
	const char **projName;
	const uint16 *BHVersion,*BHVLastCompatible;
	bool ok = false;
	BPath aoPath(path);
	
	BString alTxt(aoPath.Leaf());		// chaîne utilisée pour générer des alertes d'erreur

	if ((addOnImage>=0) &&
		(get_image_symbol(addOnImage,"InstantiateProject",B_SYMBOL_TYPE_ANY,&instFunc) == B_OK) &&
		(get_image_symbol(addOnImage,"projectName",B_SYMBOL_TYPE_ANY,(void**)&projName) == B_OK) &&
		(get_image_symbol(addOnImage,"BHVersion",B_SYMBOL_TYPE_ANY,(void**)&BHVersion) == B_OK) &&
		(get_image_symbol(addOnImage,"BHVLastCompatible",B_SYMBOL_TYPE_ANY,(void**)&BHVLastCompatible) == B_OK))
	{
		if ((*BHVLastCompatible) > bhv_application)
		{
			if (alert)
			{
				alTxt << " " << T("isn't compatible with this (old) version of BeHappy. Please download a more recent version of BeHappy.");
				BAlert *myAlert = new BAlert("BeHappy",alTxt.String(),"OK",
					NULL,NULL,B_WIDTH_AS_USUAL,B_WARNING_ALERT);
				myAlert->Go();
			}
		}
		else if ((*BHVersion) < bhv_last_addon)
		{
			if (alert)
			{
				alTxt << " " << T("is too old, and isn't compatible with this version of BeHappy. Please find a more recent version of the add-on.");
				BAlert *myAlert = new BAlert("BeHappy",alTxt.String(),"OK",
					NULL,NULL,B_WIDTH_AS_USUAL,B_WARNING_ALERT);
				myAlert->Go();
			}
		}
		else
		{
			ok = true;
			if (pName != NULL)
				*pName=*projName;
		}					
	}
	else	
	{
		if (alert)
		{
			alTxt << " " << T("is an invalid BeHappy add-on.");
			BAlert *myAlert = new BAlert("BeHappy",alTxt.String(),"OK",
				NULL,NULL,B_WIDTH_AS_USUAL,B_WARNING_ALERT);
			myAlert->Go();
		}
	}
	
	if (addOnImage>=0)
		unload_add_on(addOnImage);

	return ok;
}

const BPath *BeHappy::GetAddOnPath(const char *name)
{
	// boucle dans tous les add-ons
	for (int i=0; i<addOnsNames.CountItems(); i++)
	{
		const BString *str = (BString*)addOnsNames.ItemAt(i);
		if (*str == name)
			return((BPath*)addOnsPaths.ItemAt(i));
	}
	
	return NULL;
}

void BeHappy::InitLocale()
{
	// d'abord on cherche le dossier de l'application
	app_info l_myInfo;
	be_app->GetAppInfo(&l_myInfo);
	BEntry l_appEntry(&(l_myInfo.ref));
	BDirectory l_LocDir;
	l_appEntry.GetParent(&l_LocDir);
	
	// puis on construit le chemin du fichier de localisation
	BPath l_LocPath(&l_LocDir,"BeHappy.loc");
	
	// et enfin on crée l'objet
	g_Locale = new Locale(l_LocPath.Path());
	
	// on met la bonne langue
	BString *l_Language = prefs->FindString("English","Language");
	g_Locale->ChangeLanguage(l_Language->String());
	
	// initialisation du handler pour surveiller le fichier de localisation
	BEntry l_LocEntry(l_LocPath.Path());
	m_LanguageHandler = new LanguageHandler();
	AddHandler(m_LanguageHandler);
	m_LanguageHandler->InstallNodeMonitor(&l_LocEntry);
}

bool BeHappy::QuitRequested()
{
	// on demande au moniteur de se charger des fenêtres, et on attend leur fermeture
	thread_id l_Thread = m_Monitor->Thread();
	m_Monitor->CloseAll();
		
	if (l_Thread > 0)
	{
		status_t l_Dummy;
		wait_for_thread(l_Thread,&l_Dummy);
	}
		
	// et on finit le ménage		
	RemoveHandler(m_LanguageHandler);
	delete m_LanguageHandler;
	
	eraser->PostMessage(B_QUIT_REQUESTED);

	return true;
}

const char *BeHappy::AddOnTranslate(const HTMLProject *proj, const char *string) const
{
	static MainWindow *s_LastWindowUsed = NULL;
		// pointeur vers la dernière fenêtre qui a fait la traduction. Avec un peu de
		// chance ce sera encore la même
	
	// on regarde si cette dernière fenêtre est la bonne
	if ((s_LastWindowUsed != NULL) && s_LastWindowUsed->HTMLProjectIs(proj))
		// oui!
		return s_LastWindowUsed->AddOnTranslate(string);
	
	// on va boucler dans toutes les fenêtres
	for (int i=0; i<CountWindows(); i++)
	{
		s_LastWindowUsed = dynamic_cast<MainWindow*>(WindowAt(i));

		// est-ce celle là?
		if ((s_LastWindowUsed != NULL) && s_LastWindowUsed->HTMLProjectIs(proj))
			// oui!
			return s_LastWindowUsed->AddOnTranslate(string);
	}
	
	// on n'a pas trouvé la fenêtre
	return string;
}

void BeHappy::WindowClosed()
{
	// une fenêtre se ferme... on prévient le moniteur
	m_Monitor->PostMessage('Clos');
}
