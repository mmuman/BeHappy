/*
* Add-on BeBook pour BeHappy
*
* (c) 1999 Sylvain Tertois
*
*/

#include "BHBeBook.h"
#include "HappyLink.h"
#include "HTMLFile.h"
#include "BHAdd-ons.h"
#include "BeBookPathWin.h"
#include <stdio.h>
#include <Alert.h>
#include <Roster.h>

const uint16 BHVersion = BHAO_VERSION;
const uint16 BHVLastCompatible = BHAO_LCVERSION;

// Liste des 'topics' à ignorer
/// Dans l'ordre alphabétique!!!!
const unsigned int numIgnoreTopics = 14;
const char* ignoreTopics[numIgnoreTopics] = {
	"Archived Fields",
	"Constants",
	"Constants and Defined Types",
	"Constructor",
	"Constructor and Destructor",
	"Data Members",
	"Defined Types",
	"Hook Function",
	"Hook Functions",
	"Member Functions",
	"Operators",
	"Scripting Support",
	"Static Functions",
	"see" };

// macro pour les traductions
#define T(x) Translate(x)
	
HTMLProject *InstantiateProject(BMessage *archive)
{
	if (archive==NULL)
		return new BHBeBook;
	else
		return new BHBeBook(archive);
}

const char *projectName="Be Book";

BHBeBook::BHBeBook(BMessage *archive)
	: HTMLProject(archive)
{
}

BHBeBook::BHBeBook()
{
}

const unsigned int *BHBeBook::GiveConfig() const
{
	static const unsigned int browseConf[] = { BW_SECONDLIST, BW_SECONDLIST|BW_NEXTBUTTON,
				BW_SEARCHTXTV|BW_CONTAINS|BW_INFOTEXT, BW_SEARCHTXTV|BW_CONTAINS|BW_INFOTEXT};
	return(browseConf);
}

static const char *c_DefaultBeBookPath = "/boot/beos/documentation/Be Book";

/*bool BHBeBook::FindHome()
{
	SetHome(BDirectory(c_DefaultBeBookPath));

	return true;
}
*/

bool BHBeBook::FindHome()
{
	const char *l_BeBookPath;
	
	// ouverture d'une fenêtre de choix de chemin pour le BeBook
	l_BeBookPath = (new BeBookPathWin(this))->ChoosePath();
	
	if (l_BeBookPath != NULL)
	{
		SetHome(BDirectory(l_BeBookPath));
		delete l_BeBookPath;
		
		return true;
	}
	else
	{
		// l'utilisateur veut spécifier manuellement l'emplacement du BeBook
		SetHome(BDirectory(c_DefaultBeBookPath));
		return false;
	}
}

void BHBeBook::Update(HappyLink*)
{
	Clean();
	
	HTMLLabel *index = new HTMLLabel(this,"index.html","","Be Book",0);
	
	// création de la happylist
	happyList = new HappyLink(this,projectName,index);
	
	byKit = new HappyLink(this,"By Kit",index);
	happyList->AddChild(byKit);
	HappyLink *others = new HappyLink(this,T("Other"),index);
	
	byInheritance = new HappyLink(this,"By Inheritance",index);
	happyList->AddChild(byInheritance);
	
	functionsIndex = new HappyLink(this,"Functions index",index);
	happyList->AddChild(functionsIndex);
	
	topicsIndex = new HappyLink(this,"Topics index",index);
	happyList->AddChild(topicsIndex);
	delete index;
	
	// détermination (approximative) du nombre de fichiers à éplucher
	int nFiles = 0;
	{
		BPath *l_Path = GetPath("");
		BDirectory mainDir(l_Path->Path());
		delete l_Path;
		
		BEntry entry;
		BDirectory kitDir;
		
		while(mainDir.GetNextEntry(&entry) == B_OK)
		{
			if (kitDir.SetTo(&entry) == B_OK)
				nFiles+=kitDir.CountEntries();
		}
	}
	NumPrint(nFiles/2);	// chaque classe a un "summary"
			
	// épluchage du fichier d'index
	HTMLFile mainFile(this,"index.html");
	
	HTMLBeacon *l_BookTypeBeacon = new HTMLBeacon("<body",">");
	mainFile.AddBeacon(l_BookTypeBeacon);
	mainFile.AddBeacon(new HTMLBeacon("Updates"));
	mainFile.Search();

	// on détermine si c'est un BeBook moderne ou vieux
	BString l_Str(l_BookTypeBeacon->FoundString());
	m_NewBeBook = l_Str.IFindFirst("link") >= 0;
		
	// on cherche tous les kits
	BList kits;
	mainFile.GetLists(1,2,&kits,NULL);
	
	unsigned int n = kits.CountItems();
	
	for (unsigned int i=0; i<n; i++)
	{
		HTMLLabel *l = (HTMLLabel*)kits.ItemAt(i);
		HappyLink *happyKit = new HappyLink(this,l->LabelText(),l);
		
// épluchage d'un kit
		DoKit(l,happyKit,others);
	}
	byKit->AddChild(others);
}

void BHBeBook::DoKit(HTMLLabel *l,HappyLink *happyKit,HappyLink *others, bool addToKits)
{
	HappyLink *l_BView = NULL;	// lien vers BView, ça servira pour plus tard. En effet BView est déparée en plusieurs fichiers. Il faut les regrouper

	BString n = T("In:");
	n << " ";
	n << l->LabelText();
	n << "...";
	Print(n.String());
	n = l->LabelText();

	if ((n=="Deskbar") || (n=="Drag and Drop") || (n=="Background Images"))
	{
		// il n'y a pas de sommaire et c'est pas un vrai kit: à classer dans "autres"
		others->AddChild(happyKit);
		DoClass(l,happyKit);
	}
	else if (!m_NewBeBook && (n=="Release Notes"))
	{
		// les Release Notes chient dans les vieux BeBooks
	}
	else			
	{
		if (addToKits)
		{
			if ((n.IFindFirst("Kit") == B_ERROR) && (n.IFindFirst("Server") == B_ERROR))
				others->AddChild(happyKit);
			else
				byKit->AddChild(happyKit);
		}
		
		HTMLFile *kitFile = l->GetFile();
		
		HTMLBeacon *startBeacon = new HTMLBeacon("<H1>");
		kitFile->AddBeacon(startBeacon);
		kitFile->AddBeacon(new HTMLBeacon("<!--TOP LINKS-->"));
		kitFile->Search();

		BList classes;
		if (startBeacon->Found())
			kitFile->GetLists(1,2,&classes,NULL);
		else
			kitFile->GetLists(0,1,&classes,NULL);
		
		unsigned int nc = classes.CountItems();

// on stoque ici la dernière classe épluchée, pour gérer les classes qui sont décrites dans plusieurs
// fichiers (genre BView)
		BString l_LatestClassName="*{#fgh";	// pour être sûr que cette chaîne ne sera pas
							// trouvée dans le nom de la première classe
							// :-)
		HappyLink *l_LatestClass=NULL;
		
		for (unsigned int j=0; j<nc; j++)
		{
			HTMLLabel *lc = (HTMLLabel*)classes.ItemAt(j);
		
// on ne prend la classe en compte que si son nom est non vide
			BString l_Name = lc->LabelText();
			if (l_Name.Length()==0)
				continue;
				
			BString l_FileName = lc->FileName();
			
				
// on regarde si ce n'est pas un fichier qui décrit la même classe que le fichier d'avant
// il y a deux conditions:
// * soit le nom est "Scripting and Archival" (bug du BeBook dans BView)
// * soit le nom commence par le nom de la classe, avec un espace ou un ':' après
// * pour la dernière version du BeBook, il y a 'howto', et la BView qui est séparée en plein de petits morceaux
			if ((l_Name == "Scripting and Archival") || 
				((l_Name.FindFirst(l_LatestClassName) == 0) &&
					(l_Name[l_LatestClassName.CountChars()] == ' ') ||
					(l_Name[l_LatestClassName.CountChars()] == ':')) ||
				(l_Name == "howto"))
			{
				// oui, c'est bien le cas...
				// on reprend la classe précédente
				DoClass(lc,l_LatestClass,false);
			}
			else if ( (l_BView != NULL) && (l_FileName.FindFirst("/View") > 0))
			{
				// si le nom du fichier commence par View c'est que c'est une BView
				// on ajoute le contenu à la BView
				DoClass(lc,l_BView,false);
			}
			else
			{
				// ce n'est pas le cas, on crée une nouvelle classe
				l_LatestClass = new HappyLink(this,lc->LabelText(),lc);
				happyKit->AddChildSorted(l_LatestClass);
				
				l_LatestClassName = l_Name;
				DoClass(lc,l_LatestClass);
				
				// si c'est une BView, on garde de plus un pointeur vers le happylink
				if (l_LatestClassName == "BView")
					l_BView = l_LatestClass;
			}
		}

		delete kitFile;
	}
}

void BHBeBook::DoClass(HTMLLabel *lc, HappyLink *happyClass, bool doInheritance)
{
	BString n = T("Parsing");
	n << " ";
	n << lc->LabelText();
	n << "...";
	Print(NULL,n.String());

	HTMLFile *classFile = lc->GetFile();
	HTMLBeacon *hb = new HTMLBeacon(">Derived from:",(m_NewBeBook?"<br>":"<P>"));
	HTMLBeacon *hb2 = new HTMLBeacon("Mix-in classes:",(m_NewBeBook?"<br>":"<P>"));
	classFile->AddBeacon(hb);
	classFile->AddBeacon(hb2);
					
	classFile->Search();
	BList labels;
	classFile->GetLists(0,3,NULL,&labels);
					
	unsigned int nl = labels.CountItems();

	// ajout dans la liste par classe					
	for (unsigned int k=0; k<nl; k++)
	{
		HTMLLabel *myLabel = (HTMLLabel*)labels.ItemAt(k);
		if (TestMethodName(myLabel->LabelName()))
		{
			HappyLink *happyMethod = new HappyLink(this,myLabel->LabelName(),myLabel);
			happyClass->AddChildSorted(happyMethod);
		}							
	}

	// est-ce vraiment une classe, ou est-ce un 'topic'?
	if (((hb->Found()) && (BString("Global Functions") != lc->LabelText())) || !doInheritance)
		// ce truc bizarre compense un bug du BeBook... dans The Media Kit/functions.html
		// ils ont mis un "Derived from' au lieu de 'Declared in'					
	{
		// ajout dans la liste par héritage
		HappyLink *me;
		if (doInheritance)
		{
			/// on commence par voir si le lien n'a pas déjà été mis
			me = byInheritance->Search(lc->LabelText(),2);
			if (me != NULL)
			{
				byInheritance->RemoveChild(me);
				me->UpdateLink(this,lc);
			}
			else
			{
				// sinon on en crée un nouveau
				me = new HappyLink(this,lc->LabelText(),lc);
			}
		}
		else
			// on prend le dernier lien qui avait été mis dans la liste par héritage
			me = m_LatestByInheritanceClass;
						
		/// ajout des méthodes
		for (unsigned int k=0; k<nl; k++)
		{
			HTMLLabel *myLabel = (HTMLLabel*)labels.ItemAt(k);
			if (myLabel->LabelName()[0] != 0)
			{
				bool ok = true;		// utilisé tout à l'heure pour l'ajout à l'index
				
				// Il y a juste un truc à éviter: toutes les classes ont un Label au début qui a
				// le même nom que la classe, au tout début du fichier. Alors je ne suis pas contre,
				// mais ça se répète un peu quand on fait des recherches dans l'héritage (avec
				// Next) Donc si c'est le truc en question, j'ajoute un espace à la fin
				BString happyMethodName(myLabel->LabelName());
				if (happyMethodName == me->name)
				{
					happyMethodName += " ";
					ok = false;
				}

				// si tout va bien, on ajoute le lien
				if (TestMethodName(happyMethodName.String()))
				{
					HappyLink *happyMethod = new HappyLink(this,happyMethodName.String(),myLabel);
					me->AddChildSorted(happyMethod);
								
					// et ajout à l'index
					BString indexText(myLabel->LabelName());
					happyMethod = new HappyLink(this,indexText.String(),myLabel,lc->LabelText());

					// doit-on le mettre dans l'index fonctions ou topics?
					HappyLink *idx = functionsIndex;
					BString nm(myLabel->LabelName());

					if ( (nm.Length() > 2 ) && (nm.FindLast("()") != nm.Length()-2) && (nm[0] != '~') )
					{
						// à priori c'est un topic...
						// on vérifie qu'il ne fait pas partie de la liste interdite
						// remplacement des '_' par des ' '
						nm.Replace('_',' ',10);
						for(unsigned int nComp=0; nComp<numIgnoreTopics; nComp++)
						{
							int ic = nm.Compare(ignoreTopics[nComp]);
							if (ic==0)
							{
								// il est interdit!
								ok = false;
								break;
							}
							
							if (ic<0)
								// on a pas besoin de continuer, on est sûrs qu'on a passé la
								// chaîne dans la liste d'interdits
								break;
						}
						idx = topicsIndex;
					}

					if (ok)
						idx->AddChildSorted(happyMethod,SORT_ALL);
				}
			}							
		}

		if (doInheritance)
		{
			/// on trouve de quelle(s) classe(s) on hérite
			BString fs= hb->FoundString();
			ClearStr(fs);
			if (hb2->Found() && (fs == "none"))
				fs = hb2->FoundString();
				
			fs.RemoveAll("public ");
			fs.RemoveAll("virtual ");
			BString father;
							
			for(;fs!="";)
			{
				ClearStr(fs);
			
				int end = fs.FindFirst(',');
				if (end > 0)
				{
					fs.CopyInto(father,0,end);
					fs.Remove(0,end+1);
				}
				else
				{
					end = fs.FindFirst(' ');
					if (end >= 0)
					{
						fs.CopyInto(father,0,end);
					}
					else
					{
						father=fs;
					}
					fs="";
				}
				ClearStr(father);
				if (father=="")
				{
					break;
				}
			
				/// si l'objet n'hérite de rien, on le met en racine
				if ((father=="none") || (father=="(none)"))
				{
					byInheritance->AddChildSorted(new HappyLink(*me),SORT_PARENTS_ONLY);
				}
				else
				{
					/// on cherche si le père est déjà mis dans la liste
					BList list(2);
					HappyLink *f;
					byInheritance->Search(father,1000,&list);
					f = (HappyLink*)list.ItemAt(0);
								
					if (f == NULL)
					{
						//// il n'existe pas... on va le créer et le mettre dans la racine
						f = new HappyLink(this,father.String(),NULL);
						byInheritance->AddChildSorted(f,SORT_PARENTS_ONLY);
						m_LatestByInheritanceClass = new HappyLink(*me);
						f->AddChildSorted(m_LatestByInheritanceClass,SORT_PARENTS_ONLY);
							// on en profite pour mettre à jour le pointeur vers la dernière classe ajoutée
							// dans l'index
					}
					else
					{
						//// sinon on va boucler pour toutes les occurences du père et on s'ajoute dans les fils
						unsigned int n=1;
						do
						{
							m_LatestByInheritanceClass = new HappyLink(*me);
							f->AddChildSorted(m_LatestByInheritanceClass,SORT_PARENTS_ONLY);
								// on en profite pour mettre à jour le pointeur vers la dernière classe ajoutée
								// dans l'index
						}
						while((f = (HappyLink*)list.ItemAt(n++)) != NULL);
					}
				}
			}
			delete me;
		}
	}
	else
	{		// c'est un "topic"
		for (unsigned int k=0; k<nl; k++)
		{
			HTMLLabel *myLabel = (HTMLLabel*)labels.ItemAt(k);
			if (myLabel->LabelName()[0] != 0)
			{
				// ajout à l'index des topics
//				BString indexText(myLabel->LabelText());
				BString indexText(myLabel->LabelName());
				
				if (indexText.CountChars()>50)
					continue;			// pour éviter les phrases de 3km de long (bugs de BeBook)

				if (!TestMethodName(indexText.String()))
					continue;			// pareil si c'est un truc louche en double
					
				// dans certains cas (fonctions globales) il faut en fait ajouter le truc à l'index des fonctions
				HappyLink *idx = topicsIndex;
				
/*				BString nm(myLabel->LabelText());
				if ( nm.FindFirst('(') >= 0 )
*/				if ( indexText.FindFirst('(') >= 0 )
					idx = functionsIndex;
					
				idx->AddChildSorted(new HappyLink(this,indexText.String(),myLabel,lc->LabelText()),SORT_ALL);
			}							
		}
	}
			
	delete classFile;
}

bool BHBeBook::TestMethodName(const char *name)
{
	if (name[0] == 0)
		return false;						// si le nom est vide, il jarte

	// si le nom est de la forme 'nnnnn:' au début (où les n sont des chiffres)
	// il faut le virer, parce que c'est un double
	if (name[5]==':')
	{
		/// on a déjà le ':', on cherche les chiffres maintenant
		bool chiffres = true;
		for (unsigned int ch=0; ch<5; ch++)
		{
			char c=name[ch];
			if (c<'0' || c>'9')
				chiffres = false;
		}
		
		/// s'il n'y a que des chiffres avant le ':', on passe cette 'méthode'
		if (chiffres)
			return false;
	}
	
	// si le nom commence par un '+' avec que des chiffres après, c'est pareil
	if (name[0]=='+')
	{
		const char *p = name+1;
		while (*p>='0' && *p<='9')
		{
			if (*++p == 0)
				return false;
		}
	}
	
	// sinon, ben c'est ok
	return true;
}

void BHBeBook::ClearStr(BString &src)
{
	while ((src.Length()!=0) && (src[0]==' '))
		src.Remove(0,1);
	
	while ((src.Length()!=0) && (src[src.Length()-1]==' '))
		src.Truncate(src.Length()-1);
}

void BHBeBook::About() const
{
	char tmp[100];
	BString l_Format = T("BeBook add-on, version %x.%2.2x");
	l_Format << "\n";
	l_Format << T("by Sylvain Tertois (e)");
	sprintf(tmp,l_Format.String(),BHVersion>>8,BHVersion&0xff);
	BAlert *info = new BAlert("BeHappy:BeBook",tmp,T("Send email"),T("OK"));
	if (info->Go() == 0)
	{
		char *argv[2]= { "app", "mailto:behappy@becoz.org" };
	
		be_roster->Launch("text/x-email",2,argv);
	}
}
