/*
* Projet HTML
*
* lecture dans un fichier HTML
*
*/

#include "HTMLFile.h"
#include "HTMLProject.h"
#include <stdio.h>
#include "TranslateEntities.h"

HTMLFile::HTMLFile(HTMLProject *p, const char *fname)
{
	proj = p;
	name.SetTo(fname);
	searchDone = false;
	
	beacon = new BList(4);
	links = new BList(10);
	labels = new BList(10);
}

HTMLFile::~HTMLFile()
{
	HTMLBeacon *b;
	HTMLLabel *l;
	
	while ((b = (HTMLBeacon*)beacon->RemoveItem((int32)0)) != NULL)
		delete b;

	while ((l = (HTMLLabel*)links->RemoveItem((int32)0)) != NULL)
		delete l;

	while ((l = (HTMLLabel*)labels->RemoveItem((int32)0)) != NULL)
		delete l;
	
	delete beacon;
	delete links;
	delete labels;
}

bool HTMLFile::operator==(const HTMLFile &f) const
{
	return (name==f.name);
}

void HTMLFile::GetLists(unsigned int beacon1, unsigned int beacon2, BList *llk, BList *llb)
{
	if (!searchDone)
		Search();

	unsigned int n = links->CountItems();
	if ((n!=0) && (llk != NULL))
	for(unsigned int i=0; i<n; i++)
	{
		HTMLLabel *lab = (HTMLLabel*)links->ItemAt(i);
		if (( lab->beacon >= beacon1) && ( lab->beacon < beacon2 ))
			llk->AddItem(lab);
	}
	
	n = labels->CountItems();
	if ((n!=0) && (llb != NULL))
	for(unsigned int i=0; i<n; i++)
	{
		HTMLLabel *lab = (HTMLLabel*)labels->ItemAt(i);
		if (( lab->beacon >= beacon1) && (lab->beacon < beacon2 ))
			llb->AddItem(lab);
	}
}

void HTMLFile::GetLists(HTMLBeacon *beacon1,HTMLBeacon *beacon2, BList *llk, BList *llb)
{
	int b1,b2;
	
	if ((b1 = beacon->IndexOf(beacon1)) <0)
		b1=0;
	
	if ((b2 = beacon->IndexOf(beacon2)) <0)
		b2=1000;
	
	GetLists((unsigned int)b1,(unsigned int)b2,llk,llb);
}

static const char *slabel1="<A ";
static const char *slabel2=">";
static const char *slabel3="</A";

status_t HTMLFile::Search()
{
	typedef enum { NOTHING, IDENTIFYING, BEGIN_STRING, EXTRACT_WORD, EXTRACT_STRING } statusEnum;
		// tâche en cours de la machine d'état
	typedef enum { LABEL = 1, BEACON = 2 } stringType;
		// balise que la machine d'état est en train de reconnaître
		// (ce sont des puissances de 2)
	
	const unsigned int numString = 2;
	const char *strings[numString] = { slabel1, NULL };
		// chaines de caractères à identifier
	const char *endString=NULL;	
	
	// gestion des balises: initialisation à la première balise
	unsigned int bnum = 0;
	HTMLBeacon *bec = (HTMLBeacon*)beacon->ItemAt(bnum++);
	
	if (bec != NULL)
		strings[1] = bec->str1.String();
	
	// ouverture et chargement du fichier	
	BPath *path = proj->GetPath(name.String());
	if (path->InitCheck() != B_OK)
		return B_NAME_NOT_FOUND;
		
	BFile htmlFile(path->Path(),B_READ_ONLY);
	if (htmlFile.InitCheck() != B_OK)
		return htmlFile.InitCheck();
	
	char *data;
	unsigned int pointer=0,idpointer=0;
	off_t size;
	htmlFile.GetSize(&size);
	
	data = new char[size];
	htmlFile.Read(data,size);
	
	// épluchage du fichier
	statusEnum status;
	status = NOTHING;
	off_t p1=0,p2=0,p3=0,p4=0;
	unsigned int strp[numString],strpe=0;
	int st=0;
	
	while(pointer<size)
	{
		char ch = data[pointer++];
		if ( ch>='a' && ch<='z')
			ch -= ('a'-'A');
		if (ch==0xa || ch==0xd)
			ch=' ';
			
		switch (status)
		{
			case NOTHING:
				// rien n'a été encore identifié, on cherche la chaîne
			{
				st=(statusEnum)0;
				for(unsigned int i=0; i<numString; i++)
					if((strings[i] != NULL) && (ch==*strings[i]))
					{
						strp[i] = 1;
						st |= (1<<i);
					}
				
				if (st!=0)
				{
					idpointer = pointer;
					status = IDENTIFYING;
				}
			}
			break;
			
			case IDENTIFYING:
				// on a trouvé un début de chaine, on continue
			{
				for(unsigned int i=0; i<numString; i++)
					if (st&(1<<i))
					{
						const char tch = strings[i][strp[i]++];
						if (tch == 0)
							// on a trouvé!!!
						{
							st = (1<<i);
							status = BEGIN_STRING;
							pointer--;
						}
						else if ((ch!=tch) && (ch!=' '))
							// c'est pas cette chaîne
							st &= ~(1<<i);
						
						else if ((ch==' ') && (tch!=' '))
							// on saute l'espace
							strp[i]--;
					}
				
				if (st==0)
				{
					status = NOTHING;
					pointer = idpointer;
				}
			}
			break;
					
			case BEGIN_STRING:
				// on cherche le début d'une chaine
			{
				if (ch!=' ')
					if (ch=='"')
					{
						status=EXTRACT_STRING;
						endString="\"";
						switch(st)
						{
							case LABEL:
								if(strings[0] == slabel1)
								{
									p1=pointer;
									endString=slabel2;
								}
								else
								{
									p3=pointer-1;
									endString=slabel3;
								}
								break;
							
							case BEACON:
								if (*(bec->str2.String()) != 0)
								{
									p1=pointer-1;
									endString=bec->str2.String();
								}
								else
									p1=pointer;
								break;
						}
					}
					else
					{
						status=EXTRACT_WORD;
						pointer--;
						switch(st)
						{
							case LABEL:
								if(strings[0] == slabel1)
								{
									p1=pointer;
									endString=slabel2;
								}
								else
								{
									p3=pointer;
									endString=slabel3;
								}
								status=EXTRACT_STRING;
								break;
							
							case BEACON:
								p1 = pointer;
								if (*(bec->str2.String()) != 0)
								{
									status=EXTRACT_STRING;
									endString=bec->str2.String();
								}
						}
					}
			}
			break;
			
			case EXTRACT_WORD:
				// on extrait un mot (->on attend un espace)
				if ((ch==' ') || (ch=='>')) 
				{
					p2 = --pointer;
					status=NOTHING;
					switch(st)
					{
						case BEACON:
// la ligne suivante cause un internal compiler error si on met des optimisations...
							FoundBeacon(data,p1,p2,bec);
							strings[0]=slabel1;
							bec = (HTMLBeacon*)beacon->ItemAt(bnum++);
							if (bec != NULL)
								strings[1] = bec->str1.String();
							else
								strings[1] = NULL;
							break;
					}
				}
			break;
			
			case EXTRACT_STRING:
				// on extrait une chaîne (->on attend endString )
				if (endString[strpe++]==0)
				{
					strpe=0;
					status=NOTHING;
					pointer-=strlen(endString)+1;
					switch(st)
					{
						case LABEL:
							if (strings[0] == slabel1)
							{
								p2 = pointer;
								strings[0]=slabel2;
								strings[1]=NULL;
							}
							else
							{
								p4 = pointer;
// la ligne suivante cause un internal compiler error si on met des optimisations...
								FoundAToken(data,p1,p2,p3,p4,bnum);
								strings[0]=slabel1;
								if (bec != NULL)
									strings[1] = bec->str1.String();
								else
									strings[1] = NULL;
							}

							break;
						
						case BEACON:
							p2=pointer;
// la ligne suivante cause un internal compiler error si on met des optimisations...
							FoundBeacon(data,p1,p2,bec);
							strings[0]=slabel1;
							bec = (HTMLBeacon*)beacon->ItemAt(bnum++);
							if (bec != NULL)
								strings[1] = bec->str1.String();
							else
								strings[1] = NULL;
							break;
					
					}
				}
				else 
					if (ch!=endString[strpe-1])
						strpe=0;
				break;		
		}
	}
	
	searchDone = true;
	
	delete data;
	
	return B_OK;
}

void HTMLFile::FoundBeacon(char *data,int p1,int p2,HTMLBeacon *b)
{
	b->foundStr.SetTo(data+p1,p2-p1);
	ChangeString(b->foundStr);
	b->found = true;
}

void HTMLFile::FoundAToken(char *data,int p1,int p2,int p3,int p4,unsigned int bnum)
{
	BString s1,s2;
	s1.SetTo(data+p1,p2-p1);
	s2.SetTo(data+p3,p4-p3);
	ChangeString(s1);
	ChangeString(s2);

	// recherche d'un label
	int n = s1.IFindFirst("name=");
	if (n>=0)
	{
		n+=5;
		// recherche de la fin du nom
		int l;
		if (s1[n]=='"')
		{
			n++;
			l = s1.FindFirst('"',n);
		}
		else
			l = s1.FindFirst(' ',n);
		if (l<0)
			l = s1.CountChars();
		
		// découpe de la partie intéressante
		BString ls;
		s1.CopyInto(ls,n,l-n);
		FoundLabel(ls,s2,bnum);
	}
	
	// recherche d'un link
	n = s1.IFindFirst("href=");
	if (n>=0)
	{
		n+=5;
		// recherche de la fin du nom
		int l;
		if (s1[n]=='"')
		{
			n++;
			l = s1.FindFirst('"',n);
		}
		else
			l = s1.FindFirst(' ',n);
		if (l<0)
			l = s1.CountChars();
		
		// découpe de la partie intéressante
		BString ls;
		s1.CopyInto(ls,n,l-n);
		FoundLink(ls,s2,bnum);
	}
}

void HTMLFile::FoundLabel(BString &s1,BString &s2, unsigned int bnum)
{
	labels->AddItem(new HTMLLabel(proj,name.String(),s1.String(),s2.String(),bnum-1));
}

void HTMLFile::FoundLink(BString &s1,BString &s2, unsigned int bnum)
{
	BString file,label;
	int i = s1.FindFirst('#');
	if (i<0)
	{
		file = s1;
		label = "";
	}
	else
	{
		s1.CopyInto(file,0,i);
		s1.CopyInto(label,i+1,s1.CountChars()-i-1);
	}

	if (file.FindFirst("http://") >=0)
		return;
	
	if(file=="")
		file = name;
	else if (file[0] != '/')
	{
		BPath *p = proj->GetPath("");
		BString projectPath = p->Path();
		projectPath += '/';
		delete p;
		
		BString relName = name;
		int i = relName.FindLast("/");
		if (i<0)
			relName = "";
		else
			relName.Remove(i+1,relName.CountChars()-i-1);
		relName.Append(file);
		
		p = proj->GetPath(relName.String());
		file = p->Path();
		file.RemoveFirst(projectPath);
		delete p;
	}

	links->AddItem(new HTMLLabel(proj,file.String(),label.String(),s2.String(),bnum-1));
}

void HTMLFile::ChangeString(BString &s) const
{
	s.ReplaceAll("%20"," ");
	s.RemoveSet("\012\015");
	
	int i;
	while ( (i=s.FindFirst('<')) >= 0 )
	{
		int f = s.FindFirst('>',i);
		if (f < 0)
			f = s.CountChars()-1;
		s.Remove(i,f-i+1);
	}
	
	// élimination des espaces de tête et de fin
	while((s.Length() != 0) && (s[0]==' '))
		s.Remove(0,1);
	
	while((s.Length() != 0) && (s[s.Length()-1]==' '))
		s.Remove(s.Length()-1,1);

	// gestion des accents et charactères spéciaux
	TranslateEntities(s);
}

void HTMLFile::Dump()
{
	unsigned int n = beacon->CountItems()+1;
	unsigned int plk=0, plb=0;
	HTMLLabel *l;
	
	printf("%ld links and %ld labels found\n",links->CountItems(), labels->CountItems());
	for (unsigned int i=0; i<n; i++)
	{
		l = (HTMLLabel*)links->ItemAt(plk++);
		while ( (l!=NULL) && (l->beacon == i))
		{
			printf("Link: file '%s', label '%s', name '%s'\n",l->fname.String(),l->lname.String(),l->ltext.String());
			l = (HTMLLabel*)links->ItemAt(plk++);
		}
		plk--;
			
		l = (HTMLLabel*)labels->ItemAt(plb++);
		while( (l!=NULL) && (l->beacon == i))
		{
			printf("Label: '%s', name '%s'\n",l->lname.String(),l->ltext.String());
			l = (HTMLLabel*)labels->ItemAt(plb++);
		}
		plb--;
		
		HTMLBeacon *b = (HTMLBeacon*)beacon->ItemAt(i);
		if (b!=NULL)
			printf("**Beacon** retreived '%s'\n",b->foundStr.String());
	}
}

HTMLBeacon::HTMLBeacon(const char *stringToFind)
{
	str1.SetTo(stringToFind);
	str1.ToUpper();
	str2.SetTo("");
	found = false;
}

HTMLBeacon::HTMLBeacon(const char *string1, const char *string2)
{
	str1.SetTo(string1);
	str1.ToUpper();
	str2.SetTo(string2);
	str2.ToUpper();
	found = false;
}

HTMLLabel::HTMLLabel(HTMLProject *p, const char *fn, const char *ln, const char *lt, unsigned int b)
{
	proj = p;
	fname.SetTo(fn);
	lname.SetTo(ln);
	ltext.SetTo(lt);
	beacon = b;
}

HTMLLabel::HTMLLabel(const HTMLLabel *l)
{
	proj = l->proj;
	fname = l->fname;
	lname = l->lname;
	ltext = l->ltext;
	beacon = l->beacon;
}

HTMLLabel::~HTMLLabel()
{
}

bool HTMLLabel::operator==(const HTMLLabel &l) const
{
	return (fname == l.fname) && (lname == l.lname) && (ltext == l.ltext) && (beacon == l.beacon);
}