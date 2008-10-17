/*
 * BeHappy
 * ©1999-2001 Sylvain Tertois
 *
 * Add-on pour le BeBook
 *
 * Fenêtre de choix de chemin pour le BeBook
 *
 */

#include "BeBookPathWin.h"
#include "HTMLProject.h"
#include "PathFinder.h"
#include <StringView.h>
#include <Button.h>
#include <RadioButton.h>

// macro pour la traduction
#define T(x) add_on->Translate(x)
 
BeBookPathWin::BeBookPathWin(HTMLProject *add_on)
	: BWindow(BRect(100,100,400,110),T("BeBook Path"),B_FLOATING_WINDOW_LOOK,
	B_MODAL_APP_WINDOW_FEEL,B_NOT_ZOOMABLE|B_NOT_RESIZABLE|
	B_ASYNCHRONOUS_CONTROLS)
{
	// diverses initialisations
	m_WaitForEndSema = create_sem(0,"BeBookPathWin wait for end");
	
	// création de la vue principale
	m_MainView = new BView(Bounds(),NULL,B_FOLLOW_ALL_SIDES,B_WILL_DRAW);
	m_MainView->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	AddChild(m_MainView);
	
	// ajout d'un texte
	BRect l_Rect = Bounds();
	BStringView *l_String = new BStringView(l_Rect,NULL,T("Please Select the path to use for the Be Book:"));
	m_MainView->AddChild(l_String);
	l_String->ResizeToPreferred();
	l_Rect = l_String->Frame();
	l_Rect.right = Bounds().right;
	
	// ajout des boutons
	m_Y = l_Rect.bottom+8;
	const float c_MarginX = 10;
	const float c_MarginY = 10;
	const float c_SizeX = Bounds().right/2-3*c_MarginX;
	l_Rect.top = l_Rect.bottom+c_MarginY;
	l_Rect.left = c_MarginX;
	l_Rect.right = l_Rect.left+c_SizeX;
	BButton *l_OKButton = new BButton(l_Rect,NULL,T("OK"),new BMessage('Ok__'),
		B_FOLLOW_LEFT | B_FOLLOW_TOP);
	m_MainView->AddChild(l_OKButton);
	
	l_Rect.left = l_Rect.right+c_MarginX;
	l_Rect.right = l_Rect.left+c_SizeX;
	BButton *l_ManualButton = new BButton(l_Rect,NULL,T("Set Manually"),
		new BMessage(B_QUIT_REQUESTED), B_FOLLOW_LEFT | B_FOLLOW_TOP);
	m_MainView->AddChild(l_ManualButton);
	
	// on compte le nombre d'enfants de la vue principale
	m_NumOfViews = m_MainView->CountChildren();
	
	// réglage de la fenêtre
	l_Rect = Bounds();
	l_Rect.bottom = l_ManualButton->Frame().bottom;
	ResizeTo(l_Rect.Width(),l_Rect.Height());
	l_OKButton->SetResizingMode(B_FOLLOW_LEFT | B_FOLLOW_BOTTOM);
	l_ManualButton->SetResizingMode(B_FOLLOW_LEFT | B_FOLLOW_BOTTOM);
	
	// on la place sous la souris...
	BPoint l_MousePos;
	uint32 l_MouseButtons;
	m_MainView->GetMouse(&l_MousePos,&l_MouseButtons,false);
	l_MousePos.x -= l_Rect.Width()/2;
	l_MousePos.y -= l_Rect.Height()/2;
	MoveBy(l_MousePos.x, l_MousePos.y);
	
	// et on l'affiche
	Show();
	
	// on lance la recherche
	m_PathFinder = new PathFinder(BMessenger(this));
}

BeBookPathWin::~BeBookPathWin()
{
	delete_sem(m_WaitForEndSema);
}

void BeBookPathWin::MessageReceived(BMessage *message)
{
	switch(message->what)
	{
		case PF_ADD_PATH:
		{
			const char *pathToAdd;
			if (message->FindString(PF_AR_PATH,&pathToAdd) == B_OK)
			{
				BRect l_Rect = Bounds();
				l_Rect.top = m_Y;
				l_Rect.bottom = l_Rect.top+10;
				BRadioButton *l_RadioButton = new BRadioButton(l_Rect,
					NULL,pathToAdd,NULL);
				m_MainView->AddChild(l_RadioButton);
				l_RadioButton->ResizeToPreferred();
				m_Y = l_RadioButton->Frame().bottom+1;
				
				// on redimensionne la fenêtre
				float l_DiffWidth = l_RadioButton->Frame().Width() -
					m_MainView->Frame().Width()+10;	// on ajoute 10 de marge
				/// si la différence est négative, on ne redimensionne pas la vue horizontalement
				if (l_DiffWidth < 0)
					l_DiffWidth = 0;
				
				ResizeBy(l_DiffWidth,l_RadioButton->Frame().Height()+1);
				
				// si c'est le premier, on le sélectionne
				if (m_MainView->CountChildren() == m_NumOfViews+1)
					l_RadioButton->SetValue(1);
			}
		}
		break;
		
		case PF_REMOVE_PATH:
		{
		}
		break;
		
		case 'Ok__':
		{
			// il faut trouver le bouton radio sélectionné
			for (int i=m_NumOfViews; i<m_MainView->CountChildren(); i++)
			{
				BRadioButton *l_RadioButton = dynamic_cast<BRadioButton*>(m_MainView->ChildAt(i));
				if ((l_RadioButton != NULL) && (l_RadioButton->Value() != 0))
				{
					// trouvé
					m_CurrentPath.SetTo(l_RadioButton->Label());
					break;
				}
			}
			
			PostMessage(B_QUIT_REQUESTED);
		}
		break;
					
		default:
			BWindow::MessageReceived(message);
		break;
	}
}

bool BeBookPathWin::QuitRequested()
{
	// on débloque l'appel ChoosePath
	release_sem(m_WaitForEndSema);
	
	// supression du looper
	m_PathFinder->PostMessage(B_QUIT_REQUESTED);

	// attente avant la supression de l'objet
	m_DeleteLocker.Lock();
	m_DeleteLocker.Unlock();

	return true;
}
	
char *BeBookPathWin::ChoosePath()
{
	// verrou pour le QuitRequested
	m_DeleteLocker.Lock();
	
	// attente de la fermeture de la fenêtre
	acquire_sem(m_WaitForEndSema);
	
	// y-a-t'il un chemin sélectionné?
	if (m_CurrentPath.Length() == 0)
	{
		// non

		// on peut débloquer le destructeur
		m_DeleteLocker.Unlock();

		// on ne rend aucun chemin...
		return NULL;
	}
	else
	{
		// oui

		// extraction du chemin
		char *string = new char[m_CurrentPath.Length()+1];
		m_CurrentPath.CopyInto(string,0,m_CurrentPath.CountChars());
		string[m_CurrentPath.Length()]=0;

		// on peut débloquer le destructeur
		m_DeleteLocker.Unlock();
		
		return string;
	}
}
		
