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

#include "MainWindow.h"
#include "NetView.h"
#include <InterfaceKit.h>
#include <Application.h>
#include <FilePanel.h>
#include <Entry.h>
#include <Path.h>
#include <Roster.h>
#include <PropertyInfo.h>
#include <NodeMonitor.h>
#include "Preferences.h"
#include "BeHappy.h"
#include "InfoWindow.h"
#include "HappyLink.h"
#include "BrowseView.h"
#include "Mover.h"
#include "Tasks.h"
#include "HTMLProject.h"
#include "Version.h"
#include "AddOnPrefFile.h"
#include "PathWindow.h"
#include "Locale.h"

#define NEW_WINDOW 'NewW'
#define OPEN_BOOK 'BkOp'
#define OB_BOOK_PATH "Path"
#define UPDATE_BOOK 'BkUp'
#define ABOUT_ADDON 'AAOn'
#define CHANGE_LIST 'CHLS'
#define CL_HAPPYLINK "Happy Link"
#define CL_CONFIG "Config"

#define WWW_HOMEPAGE 'WwHp'
#define WWW_REGISTER 'WwRg'
#define WWW_UPDATE 'WwUp'
#define WWW_BUGS 'WwBu'

// macro utilisée pour les traductions
#define T(x) g_Locale->Translate(x)

MainWindow::MainWindow(const char *openThis)
	: BWindow(BRect(50,100,800,700),"BeHappy!",B_DOCUMENT_WINDOW,0)
{
	tasks = new Tasks(this);
	
	BView *mainView = new BView(Bounds(),"main",B_FOLLOW_ALL,0);
	mainView->SetViewColor(B_TRANSPARENT_COLOR);
	AddChild(mainView);
				
	BRect viewRect(Bounds());
	viewRect.bottom = 19;
		
	/// On place le menu dans la fenêtre
	m_WindowMenuBar = NULL;
	m_AddOnLoaded = false;
	BuildMenu();

	// reste de la fenêtre
	viewRect = Bounds();
	viewRect.top = m_WindowMenuBar->Bounds().bottom+1;
	viewRect.right = 147;
	
	/// vue de gauche (index)
	leftView = new BView(viewRect,NULL,B_FOLLOW_LEFT|B_FOLLOW_TOP_BOTTOM,B_WILL_DRAW);
	leftView->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	mainView->AddChild(leftView);

	/// séparation avec la vue web
	viewRect.right = 151;
	viewRect.left = 147;
	verticalMover = new Mover(viewRect,B_FOLLOW_LEFT|B_FOLLOW_TOP_BOTTOM,true);
	mainView->AddChild(verticalMover);
	viewRect.left = 0;
	viewRect.right = 147;
	
	/// machin pour sélectionner l'index	
	viewRect.OffsetTo(0,0);
	BRect chooseRect(viewRect);
	chooseRect.top+=5;
	chooseRect.left+=10;
	chooseRect.right-=10;
	chooseRect.bottom = chooseRect.top+18;
	chooseList = new BMenuField(chooseRect,NULL,"",new BMenu(T("nothing to choose!")),B_FOLLOW_LEFT|B_FOLLOW_TOP);
	chooseList->SetDivider(0);
	leftView->AddChild(chooseList);
	
	/// bidule qui contient l'index (en dessous du truc ci-dessus)
	viewRect.top+=28;
	viewRect.right--;
	browse = new BrowseView(viewRect,tasks);
	leftView->AddChild(browse);

	/// Vue HTML
	viewRect = Bounds();
	viewRect.top += m_WindowMenuBar->Bounds().bottom+1;
	viewRect.left += 152;	
	theNetView = new NetView(viewRect,"");
	mainView->AddChild(theNetView);

	// Deux ou trois bricoles
	SetSizeLimits(300,10000,200,10000);
	prefs->SizeWindow("BeHappy Browse Window",this);
	
	projectName = "";
	project = NULL;
	projectPrefs = NULL;
	currentOpenedIndex = NULL;	
	m_AddOnLocale = NULL;
		
	Show();
	
	//doit-on ouvrir un livre?
	if (openThis != NULL)
	{
		// on ouvre le livre indiqué au constructeur
		BMessage mess(OPEN_BOOK);
		mess.AddString(OB_BOOK_PATH,openThis);
		PostMessage(&mess);
	}
	else
	{
		// y-a-t'il dans les préférences le nom du dernier livre utilisé?
		BString *book = prefs->FindString("","Last Opened Project");
		if (*book!="" && BeHappy::CheckAddOn(book->String(),false))
		{	// oui
			BMessage mess(OPEN_BOOK);
			mess.AddString(OB_BOOK_PATH,book->String());
			PostMessage(&mess);
		}
		else
		{	// non
			// ouverture de l'écran de présentation
			PostMessage(B_ABOUT_REQUESTED);
		}
	}
}

MainWindow::~MainWindow()
{
	SaveMoversPosition();
	SaveBookName();

	prefs->CloseWindow(this);
	tasks->GiveUp();
	thread_id l_TasksThread = tasks->Thread();
	tasks->PostMessage(B_QUIT_REQUESTED);
	status_t l_Dummy;
	wait_for_thread(l_TasksThread,&l_Dummy);
	
	if (project != NULL)
	{
		delete project;
		unload_add_on(currentProject);
		project = NULL;
		
		delete projectPrefs;
		projectPrefs = NULL;
		
		delete m_AddOnLocale;
		m_AddOnLocale = NULL;
	}
	
	// coupure du Node Monitor
	stop_watching(BMessenger(this));
	
	// on prévient BeHappy qu'on ferme
	((BeHappy*)be_app)->WindowClosed();
}

void MainWindow::BuildMenu()
{
	BRect viewRect(Bounds());
	viewRect.bottom = 19;
	
	// on enlève le menu s'il est déjà là
	if (m_WindowMenuBar != NULL)
		m_WindowMenuBar->RemoveSelf();
	
	// Construction du menu
	m_WindowMenuBar = new BMenuBar(viewRect,NULL);
	
	/// Menu fichier
	BMenu *fileMenu = new BMenu(T("File"));
	fileMenu->AddItem(new BMenuItem(T("Spawn new window"),new BMessage(NEW_WINDOW),'N'));
	fileMenu->AddItem(new BMenuItem(T("About BeHappy"),new BMessage(B_ABOUT_REQUESTED),'B'));
	fileMenu->AddSeparatorItem();
	fileMenu->AddItem(new BMenuItem(T("Quit"),new BMessage(B_QUIT_REQUESTED),'Q'));
	m_WindowMenuBar->AddItem(fileMenu);
	
	/// Menu de choix des Add-ons
	bookMenu = new BMenu(T("Book"));
	UpdateBookMenu();
	m_WindowMenuBar->AddItem(bookMenu);

	/// menu Actions
	addOnMenu = new BMenu(T("Action"));
	updateItem= new BMenuItem(T("Update..."),new BMessage(UPDATE_BOOK),'U');
	addOnMenu->AddItem(updateItem);
	addOnMenu->AddItem(new BMenuItem(T("About add-on"),new BMessage(ABOUT_ADDON)));
	addOnMenu->SetEnabled(m_AddOnLoaded);
	m_WindowMenuBar->AddItem(addOnMenu);

	/// menu langues
	BMenu *l_LanguagesMenu = new BMenu(T("Language"));
	const BList *l_LanguagesList = g_Locale->GetLanguagesList();
	for (int i=0; i<l_LanguagesList->CountItems(); i++)
	{
		BString *l_Language = (BString*)l_LanguagesList->ItemAt(i);
		
		// construction du message qui sera envoyé à l'application en cas de changement
		// de langue
		BMessage *l_NewMessage = new BMessage(GM_LANGUAGE);
		l_NewMessage->AddString(GM_L_NAME,l_Language->String());
		
		// construction de l'item correspondant à la langue
		BMenuItem *l_Item = new BMenuItem(l_Language->String(),l_NewMessage);
		l_Item->SetTarget(be_app_messenger);
		
		l_LanguagesMenu->AddItem(l_Item);
	}
	m_WindowMenuBar->AddItem(l_LanguagesMenu);
	
	/// menu site internet
	BMenu *l_WWWMenu = new BMenu(T("Web site"));
	l_WWWMenu->AddItem(new BMenuItem(T("Homepage"),new BMessage(WWW_HOMEPAGE)));
	l_WWWMenu->AddItem(new BMenuItem(T("Register online (free!)"),new BMessage(WWW_REGISTER)));
	l_WWWMenu->AddItem(new BMenuItem(T("Check for update"),new BMessage(WWW_UPDATE)));
	l_WWWMenu->AddItem(new BMenuItem(T("Bugs and Suggestions"),new BMessage(WWW_BUGS)));
	m_WindowMenuBar->AddItem(l_WWWMenu);
	
	FindView("main")->AddChild(m_WindowMenuBar);
}

void MainWindow::MessageReceived(BMessage *message)
{
	switch (message->what)
	{
		case NEW_WINDOW:
			SaveBookName();
			be_app->PostMessage('Open');
		break;
		
		case OPEN_BOOK:
		{
			const char *path;
			message->FindString(OB_BOOK_PATH,&path);
			NewProject(path);
		}
		break;
		
		case B_ABOUT_REQUESTED:
		{
			// recherche du fichier HTML d'about
			app_info myInfo;
			be_app->GetAppInfo(&myInfo);
			BEntry entry(&(myInfo.ref));
			BPath aboutPath(&entry);
			aboutPath.GetParent(&aboutPath);
			
			// création du répertoire de l'application
			BDirectory l_AppDir(aboutPath.Path());
			BEntry l_AboutFile(&l_AppDir,T("BH"));
			// est-ce que la version locale du fichier d'about existe?
			if (l_AboutFile.Exists())
				aboutPath.Append(T("BH"));
			else
				aboutPath.Append("BH");

			theNetView->NewUrl(aboutPath.Path());
		}
		break;
				
		case UPDATE_BOOK:
			tasks->DoProjectUpdate();
		break;
		
		case ABOUT_ADDON:
			if (project!=NULL)
				project->About();
		break;
		
		case MOVER_MESSAGE:
		{
			Mover *mv;
			if (message->FindPointer(MM_VIEW,(void**)&mv) == B_OK )
			{
				float coord = message->FindFloat(MM_COORD);
				if (mv == verticalMover)
				{
					BPoint p = ConvertFromScreen(BPoint(coord,0));

					leftView->ResizeTo(p.x,leftView->Frame().Height());
					theNetView->ResizeTo(Frame().Width()-p.x-verticalMover->Frame().Width(),theNetView->Frame().Height());
					theNetView->MoveTo(p.x+verticalMover->Frame().Width()+1,theNetView->Frame().top);
					verticalMover->MoveTo(p.x,verticalMover->Frame().top);
				}
				else
					browse->ResizeList(coord);
			}
		}
		break;
		
		case CHANGE_LIST:
		{
			SaveMoversPosition();
			
			HappyLink *newList;
			if (message->FindPointer(CL_HAPPYLINK,(void**)&newList) == B_OK)
			{
				currentOpenedIndex = newList->name.String();
				
				chooseList->MenuItem()->SetLabel(AddOnTranslate(currentOpenedIndex));
				int32 mode = BW_SECONDLIST;
				
				message->FindInt32(CL_CONFIG,&mode);
				browse->NewList(newList,mode);
				
				LoadMoversPosition();
			}
		}
		break;
		
		case 'NEXT':
			browse->NextClicked();
		break;
		
		case 'IDXC':
			browse->FindInIndex();
		break;
		
		case 'IDXV':
			if(!browse->containsSearch && browse->searchTextView->TextView()->IsFocus())
				browse->FindInIndex(true);
		break;
		
		case 'IDCN':
			browse->ContainsClicked();
		break;
			
		case BH_ADDONS_RESCANNED:
		// le répertoire des add-ons a été modifié
			UpdateBookMenu();
		break;

		// changement de langue
		// ou modification d'un fichier de localisation
		case B_NODE_MONITOR:
		case GM_LANGUAGE:
		{
			BuildMenu();
			
			// pour refaire la vue de gauche, on recherche le message qui déclenche sa
			// fabrication, et on le réenvoie
/*			BMenuItem *l_Item = chooseList->Menu()->FindItem(currentOpenedIndex);
			if (l_Item != NULL)
				PostMessage(l_Item->Message());
*/
			// mise à jour de la langue pour l'add'on
			// extraction du nom de la langue
			if (m_AddOnLocale != NULL)
			{
				const char *l_LanguageName;
				if (message->FindString(GM_L_NAME,&l_LanguageName) == B_OK)
					m_AddOnLocale->ChangeLanguage(l_LanguageName);
				else
				{
					// il faut extraire la langue des préférences
					BString *l_LanguageNameString = prefs->FindString("English","Language");
					m_AddOnLocale->ChangeLanguage(l_LanguageNameString->String());
					delete l_LanguageNameString;
				}								
				UpdateBrowse();
				
			}
		}
		break;
				
		// Scripting
		case B_GET_PROPERTY:
		{
			int32 index,spWhat;
			BMessage specifier;
			const char *spProp;
			if (message->GetCurrentSpecifier(&index,&specifier,&spWhat,&spProp)==B_OK)
			{
				// dans tous les cas, on ne marche qu'avec un direct specifier
				if (spWhat != B_DIRECT_SPECIFIER)
					BWindow::MessageReceived(message);
				else
				{
					BMessage reply(B_REPLY);
					reply.AddInt32("error",B_OK);
					
					// on regarde si c'est une propriété à nous
					BString str(spProp);
					if (str == "Book")
						reply.AddString("result",projectName.String());
					else if (str == "Index")
						reply.AddString("result",currentOpenedIndex);
					else if ((str == "Topic") || (str == "ChildTopic"))
					{
						// recherche du happylink sélectionné
						HappyLink *hl1,*hl2;
						if (project->happyList->Find(&hl1,&hl2,browse->GetLinkNum()))
							reply.AddString("result",hl2->name.String());
						else
							reply.AddString("result","");
					}
					else if (str == "FatherTopic")
					{
						// recherche du happylink sélectionné, ou plus précisément son père
						HappyLink *hl1=NULL,*hl2;
						if ((project->happyList->Find(&hl1,&hl2,browse->GetLinkNum())) &&
						    (hl1 != NULL))
							reply.AddString("result",hl1->name.String());
						else
							reply.AddString("result","");
					}
					else
					{
						BWindow::MessageReceived(message);
						return;
					}
					
					message->SendReply(&reply);
				}
			}
			else
				BWindow::MessageReceived(message);
		}
		break;
		
		case B_SET_PROPERTY:
		{
			int32 index,spWhat;
			BMessage specifier;
			const char *spProp;
			if (message->GetCurrentSpecifier(&index,&specifier,&spWhat,&spProp)==B_OK)
			{
				const char *newProp;
				// dans tous les cas, on ne marche qu'avec un direct specifier, et il doit y avoir
				// une chaîne de caractères
				if ((spWhat != B_DIRECT_SPECIFIER) || (message->FindString("data",&newProp) != B_OK))
					BWindow::MessageReceived(message);
				else
				{
					BMessage reply(B_REPLY);
					// on regarde si c'est une propriété à nous
					BString str(spProp);
					if (str == "Book")
					{
						const BPath *path = ((BeHappy*)be_app)->GetAddOnPath(newProp);
						if (path != NULL)
						{
							reply.AddInt32("error",B_OK);
							BMessage open(OPEN_BOOK);
							open.AddString(OB_BOOK_PATH,path->Path());
							PostMessage(&open);
						}
						else
							reply.AddInt32("error",B_BAD_INDEX);
					}
					else if (str == "Index")
					{
						// on cherche l'élément du menu qui fait l'index
						BMenuItem *index = chooseList->Menu()->FindItem(AddOnTranslate(newProp));
						if (index != NULL)
						{
							reply.AddInt32("error",B_OK);
							
							// on fait comme si l'utilisateur avait cliqué dans le menu
							index->Messenger().SendMessage(index->Message());
						}
						else
							reply.AddInt32("error",B_BAD_INDEX);
					}
					else if ((str == "Topic") || (str == "FatherTopic"))
					{
						// on cherche le happylink qui correspond
						HappyLink *link = browse->top->Search(BString(newProp));
						if (link != NULL)
						{
							reply.AddInt32("error",B_OK);
							// et on le sélectionne
							browse->SetLinkNum(link->link,browse->top);
						}
						else
							reply.AddInt32("error",B_BAD_INDEX);	
					}
					else if (str == "ChildTopic")
					{
						// on commence par chercher le père sélectionné
						HappyLink *hl1=NULL,*hl2;
						if ((project->happyList->Find(&hl1,&hl2,browse->GetLinkNum())) &&
						    (hl1 != NULL))
						{
							// on cherche le happylink qui correspond au fils
							HappyLink *link;
							if ((link = hl1->Search(BString(newProp))) != NULL)
							{
								reply.AddInt32("error",B_OK);
								// et on le sélectionne
								browse->SetLinkNum(link->link,browse->top);
							}
							else
								reply.AddInt32("error",B_BAD_INDEX);	
						}
						else
							reply.AddInt32("error",B_BAD_INDEX);	
					}
					else
					{
						BWindow::MessageReceived(message);
						return;
					}
					
					message->SendReply(&reply);
				}
			}
			else
				BWindow::MessageReceived(message);
		}
		break;
		
		// site web
		case WWW_HOMEPAGE:
		{
			char *argv[2] = { "http://www.becoz.org/beos/happy" };
			be_roster->Launch("text/html",1,argv);
		}
		break;
		
		case WWW_REGISTER:
		{
			BString l_url = "http://www.becoz.org/beos/happy/register.";
			l_url << T("en") << ".php";
			theNetView->NewUrlFull(l_url.String());
		}
		break;
		
		case WWW_UPDATE:
		{
			BString l_url = "http://www.becoz.org/beos/happy/updatecheck.";
			l_url << T("en") << ".php?";
			char l_Temp[20];
			sprintf(l_Temp,"major=%1x&minor=%02x",bhv_application >> 8,bhv_application & 0xff);
			l_url << l_Temp;
			theNetView->NewUrlFull(l_url.String());
		}
		break;			
		
		case WWW_BUGS:
		{
			char *argv[2] = { "http://www.becoz.org/beos/happy/board.php" };
			be_roster->Launch("text/html",1,argv);
		}
		break;
			
		default:
			BWindow::MessageReceived(message);
	}
}

status_t MainWindow::GetSupportedSuites(BMessage *message)
{
	message->AddString("suites","suite/vnd.STertois-BeHappy-window");
	
	static property_info prop_list[] = {
		{"Book",{B_GET_PROPERTY,B_SET_PROPERTY,0},{B_DIRECT_SPECIFIER,0},"Opened book"},
		{"Index",{B_GET_PROPERTY,B_SET_PROPERTY,0},{B_DIRECT_SPECIFIER,0},"Opened index"},
		{"Topic",{B_GET_PROPERTY,B_SET_PROPERTY,0},{B_DIRECT_SPECIFIER,0},"Selected topic"},
		{"FatherTopic",{B_GET_PROPERTY,B_SET_PROPERTY,0},{B_DIRECT_SPECIFIER,0},"Father of the selected topic"},
		{"ChildTopic",{B_GET_PROPERTY,B_SET_PROPERTY,0},{B_DIRECT_SPECIFIER,0},"Current Child topic selected"},
		{0}};
	BPropertyInfo prop_info(prop_list);
	message->AddFlat("messages",&prop_info);
	return(BWindow::GetSupportedSuites(message));
}

BHandler *MainWindow::ResolveSpecifier(BMessage *message,int32 index,BMessage *specifier,int32 what,const char *property)
{
	// les seules propriétés qu'on gère ici ce sont "Book", "Index", et "Topic"
	BString str(property);
	if ((str == "Book") || (str == "Index") || (str == "Topic") || (str == "FatherTopic") || (str == "ChildTopic"))
		return this;
	else
		return(BWindow::ResolveSpecifier(message,index,specifier,what,property));	
}

void MainWindow::UpdateBookMenu()
{
	// on commence par vider le menu book
	{
		BMenuItem *it;
		
		while ((it=bookMenu->RemoveItem((int32)0)) != NULL)
			delete it;
	}
	
	// ce booléen sera à true si le projet en cours est toujours dans le dossier add-ons
	// (ou si projectName=="", ce qui veut dire qu'on a pas encore chargé de projet)
	bool stillThere = (projectName=="");
	
	BeHappy *app = (BeHappy*)be_app;
	
	BPath *aoPath;
	BString *aoName;
	
	for (unsigned int i=0; (aoPath = (BPath*)app->addOnsPaths.ItemAt(i)) != NULL; i++)
	{
		aoName = (BString*)app->addOnsNames.ItemAt(i);

		if (*aoName == projectName)
			stillThere = true;
			
		// ajout des add-ons dans le menu
		BMessage *mess = new BMessage(OPEN_BOOK);
		mess->AddString(OB_BOOK_PATH,aoPath->Path());
		bookMenu->AddItem(new BMenuItem(aoName->String(),mess));
	}
	
	// si le projet en cours n'est plus dans le dossier Add-ons, on affiche le message about
	if (!stillThere)
	{
		Lock();
		browse->Clear();
		addOnMenu->SetEnabled(false);
		chooseList->SetEnabled(false);
		Unlock();

		PostMessage(B_ABOUT_REQUESTED);
	}
}

void MainWindow::NewProject(const char *path)
{
	BrowseView::BusyOn();

	if (project != NULL)
	{				// un projet est en cours: il faut le virer
		browse->Clear();
		delete project;
		unload_add_on(currentProject);
		project = NULL;
		
		delete projectPrefs;
		projectPrefs = NULL;
		
		delete m_AddOnLocale;
		m_AddOnLocale = NULL;
	}
	
	// on récupère le nouvel add-on
	/// d'abord son fichier de localisation
	{
		BString l_AddOnLocFile(path);
		l_AddOnLocFile << ".loc";
		m_AddOnLocale = new Locale(l_AddOnLocFile.String());
		
		// activation du node monitor sur ce fichier
		stop_watching(BMessenger(this));
		BEntry l_LocFileEntry(l_AddOnLocFile.String());
		if (l_LocFileEntry.Exists())
		{
			node_ref l_MyNodeRef;
			l_LocFileEntry.GetNodeRef(&l_MyNodeRef);
			watch_node(&l_MyNodeRef,B_WATCH_STAT,BMessenger(this));
		}
		
		// recherche de la langue actuellement utilisée
		BString *l_LanguageName = prefs->FindString("English","Language");
		m_AddOnLocale->ChangeLanguage(l_LanguageName->String());
		delete l_LanguageName;
	}
	
	/// puis l'add-on lui-même
	currentProject = load_add_on(path);
	instantiate_project InstantiateProject;
	const char **projName;
	if ((currentProject>=0) &&
		(get_image_symbol(currentProject,"InstantiateProject",B_SYMBOL_TYPE_ANY,(void**)&InstantiateProject) == B_OK) &&
		(get_image_symbol(currentProject,"projectName",B_SYMBOL_TYPE_ANY,(void**)&projName) == B_OK))
	{
		projectName = *projName;
		projectPrefs = new AddOnPrefFile(*projName);
		currentProjectPath=path;
		BMessage *archive = projectPrefs->GetIndex();

		if (archive == NULL)
		// il faut faire une update
		{
			tasks->DoProjectUpdate();	
		}
		else
		{
			project=(*InstantiateProject)(archive);
			if (!project->InitOK())
				tasks->DoProjectUpdate();
			else
				UpdateBrowse();
		}

		// mise à jour du menu 'update'
		Lock();
		BString menu(T("Update"));
		menu << " " << *projName;
		updateItem->SetLabel(menu.String());
		addOnMenu->Superitem()->SetEnabled(true);
		Unlock();
		
		// mise à jour du titre de la fenêtre
		menu = "BeHappy : ";
		menu << *projName;
		SetTitle(menu.String());
		
		m_AddOnLoaded = true;
	}
	else
	{
		// ya plus rien... on désactive le menu 'update'
		Lock();
		addOnMenu->SetEnabled(false);
		chooseList->SetEnabled(false);
		Unlock();
		
		BAlert *myAlert = new BAlert("BeHappy",T("Error while loading add-on"),"OK",NULL,NULL,B_WIDTH_AS_USUAL,B_STOP_ALERT);
		myAlert->Go();
		
		PostMessage(B_ABOUT_REQUESTED);
		
		m_AddOnLoaded = false;
	}
	
	BrowseView::BusyOff();
}

void MainWindow::UpdateProject()
{
	// préparation de la fenêtre	
	Lock();
	/// on désactive le menu de choix d'index et on vide ce qu'il y a en dessous
	 chooseList->SetEnabled(false);
	 browse->Clear();
	
	/// création de la fenêtre avec la barre de progression
	 BRect infoRect = Bounds();
	 float dx = (infoRect.Width()-250)/2, dy = (infoRect.Height()-44)/2;
	 infoRect.left += dx;
	 infoRect.right -= dx;
	 infoRect.top += dy;
	 infoRect.bottom -= dy;
	 infoRect = ConvertToScreen(infoRect);
	 InfoWindow *iw = new InfoWindow(infoRect);
	 BMessenger infoMess(iw);
	/// le messager servira au projet (il communique directement avec lui pour
	/// faire progresser la barre de ... progression)
	Unlock();

	// allocation d'un nouveau projet tout frais
	delete project;
	instantiate_project InstantiateProject;
	get_image_symbol(currentProject,"InstantiateProject",B_SYMBOL_TYPE_ANY,(void**)&InstantiateProject);
	project = (*InstantiateProject)();
	project->SetInfoMessenger(infoMess);
	
	// lancement de la mise à jour
	if (!project->FindHome())
	{
		iw->Hide();
		BMessenger pwM(new PathWindow(project));
		BMessage reply;
		
		// on attend que la fenêtre se ferme
		pwM.SendMessage('Init',&reply);
		iw->Show();
	}
	project->Update(NULL);
	/// fermeture de la fenêtre avec la barre
	iw->PostMessage(B_QUIT_REQUESTED);
	
	/// et on archive le nouvel index
	BMessage *archive;
	archive = new BMessage('ARCV');
	project->Archive(archive);

	Lock();
	// ajout du nouvel index aux préférences	
	 projectPrefs->SetIndex(archive);
	 delete archive;

	// et remise à jour de l'affichage
	 UpdateBrowse();
	 chooseList->SetEnabled(true);
	Unlock();
}

void MainWindow::UpdateBrowse()
{
	// vidage du menu de choix de la liste
	BMenu *chMenu=chooseList->Menu();
	unsigned int n=chMenu->CountItems();
	for (unsigned int i=0; i<n; i++)
		delete chMenu->RemoveItem((int32)0);
	chooseList->SetEnabled(true);

	// remplissage du choix de la liste
	HappyLink *list = project->happyList->child, *last = list;
	const unsigned int *confs = project->GiveConfig();
	
	/// le tableau confs doit être parcouru à l'envers... donc on va d'abord à la fin
	if (confs != NULL)
	{
		HappyLink *p = list;
		while(p!=NULL)
		{
			confs++;
			p = p->next;
		}
	}
	
	while (list != NULL)
	{
		BMessage *mess = new BMessage(CHANGE_LIST);
		mess->AddPointer(CL_HAPPYLINK,list);
		if (confs != NULL)
			mess->AddInt32(CL_CONFIG,*--confs);
			
		chMenu->AddItem(new BMenuItem(AddOnTranslate(list->name.String()),mess),0);
		last = list;
		list = list->next;
	}
	if (project->happyList->child != NULL)
		chooseList->MenuItem()->SetLabel(chooseList->MenuItem()->Submenu()->ItemAt(0)->Label());
	
	unsigned int mode = BW_SECONDLIST;
	if (confs != NULL)
		mode = *confs;
		
	browse->NewList(last,mode);
	currentOpenedIndex = last->name.String();
	LoadMoversPosition();
	GoTo(0);
}

void MainWindow::GoTo(unsigned int link)
{
	BString linkName = project->GiveName(link);
	theNetView->NewUrl(linkName.String());
}

void MainWindow::SaveMoversPosition()
{
	if ((projectPrefs == NULL) || (currentOpenedIndex == NULL))
		return;
		
	float vertical = verticalMover->Frame().left;
	float horizontal = browse->GetMoverPos();
	BPoint p(0,horizontal);
	p = browse->ConvertToScreen(p);
	p = ConvertFromScreen(p);

	BMessage *pprefs = projectPrefs->GetPrefs();
	if (pprefs == NULL)
		pprefs = new BMessage('BhAo');
	
	pprefs->RemoveName(currentOpenedIndex);
	pprefs->AddFloat(currentOpenedIndex,vertical);
	pprefs->AddFloat(currentOpenedIndex,p.y);
	
	projectPrefs->SetPrefs(pprefs);
	
	delete pprefs;
}

void MainWindow::LoadMoversPosition()
{
	BMessage *pprefs;
	float vertical,horizontal;
	
	if ((projectPrefs != NULL) &&
		((pprefs = projectPrefs->GetPrefs()) != NULL) &&
		(pprefs->FindFloat(currentOpenedIndex,0,&vertical) == B_OK) &&
		(pprefs->FindFloat(currentOpenedIndex,1,&horizontal) == B_OK))
	{
		delete pprefs;

		BPoint p = ConvertToScreen(BPoint(vertical,horizontal));

		BMessage mess(MOVER_MESSAGE);
		mess.AddPointer(MM_VIEW,verticalMover);
		mess.AddFloat(MM_COORD,p.x);
		PostMessage(&mess);
	
		browse->ResizeList(p.y);	
	}
}

void MainWindow::SaveBookName()
{
	if (currentProjectPath != B_EMPTY_STRING)
		prefs->AddString(currentProjectPath.String(),"Last Opened Project");
}
