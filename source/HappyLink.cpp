/*
* BeHappy
*
* gestion d'un index de liens HTML
*
*/

#include "HappyLink.h"
#include "HTMLProject.h"
#include "HTMLFile.h"
#include <stdio.h>
#include <DataIO.h>

HappyLink::HappyLink(HTMLProject *proj,const char *nam,HTMLLabel *lk,const char *inf)
{
	name = nam;

	if (lk != NULL)
		UpdateLink(proj,lk);

	if (inf != NULL)
		info = inf;
	
	next = NULL;
	child = NULL;
}

HappyLink::HappyLink()
{
	next=NULL;
	child=NULL;
}

HappyLink::HappyLink(HappyLink &hl)
{
	name = hl.name;
	link = hl.link;
	
	if (hl.next == NULL)
		next = NULL;
	else
		next = new HappyLink(*hl.next);
	
	if (hl.child == NULL)
		child = NULL;
	else
		child = new HappyLink(*hl.child);
}
	
HappyLink::~HappyLink()
{
	delete next;
	delete child;
}

void HappyLink::UpdateLink(HTMLProject *proj,HTMLLabel *lk)
{
	link = proj->AddLabel(lk);
}

void HappyLink::AddChild(HappyLink *kid)
{
	kid->next = child;
	child = kid;
}

// macro pour dire si on doit prendre en compte un lien, en fonction du mode
#define HL_OK(l) (!(( (mode == SORT_PARENTS_ONLY) && (l->child == NULL)) || ( (mode == SORT_KIDS_ONLY) && (l->child != NULL))))

void HappyLink::AddChildSorted(HappyLink *kid, SortType mode)
{
	if (child == NULL)
	{
		child = kid;
		return;
	}
	
	// on ne se met dans l'ordre qu'avec les liens sans fils
	// cas spécial si le lien doit être mis en premier
	if (HL_OK(child) && ( kid->name.ICompare(child->name) > 0 ))
	{
		kid->next = child;
		child = kid;
		return;
	}
	
	HappyLink *i;
	for(i = child; i->next != NULL; i = i->next)
		if ( HL_OK(i->next) && (kid->name.ICompare(i->next->name) > 0))
			break;
	
	kid->next = i->next;
	i->next = kid;
}

bool HappyLink::RemoveChild(HappyLink *kid)
{
	HappyLink *p = child;
	
	if (p==kid)
	{
		child = kid->next;
		kid->next = NULL;
		return true;
	}
	
	for(;;)
	{
		if (p==NULL)
			return false;
		
		if (p->next==kid)
		{
			p->next = kid->next;
			kid->next = NULL;
			return true;
		}
		
		p = p->next;
	}
}		
	
void HappyLink::Dump(unsigned int level)
{

	printf("%d:[%s]\n",level,name.String());
		
	if (child != NULL)
	{
		printf("{\n");
		child->Dump(level+1);
		printf("}\n");
	}
	
	if ((next != NULL) && (level != 0))
		next->Dump(level);
}		

#define HLA_NEXT	0x01
#define HLA_CHILD	0x02
status_t HappyLink::Archive(BDataIO *archive) const
{
	// numéro du lien
	uint32 l = link;
	archive->Write(&l,sizeof(uint32));
	
	// nom du lien
	uint16 s = name.Length();
	archive->Write(&s,sizeof(uint16));
	archive->Write(name.String(),s);
	
	// infos
	s = info.Length();
	archive->Write(&s,sizeof(uint16));
	archive->Write(info.String(),s);
	
	// écriture des autres
/*	uint16 others = 0;
	if (next!=NULL)
		others |= HLA_NEXT;
	if (child!=NULL)
		others |= HLA_CHILD;
	archive->Write(&others,sizeof(uint16));
	
	if (next != NULL)
		next->Archive(archive);

	if (child != NULL)
		child->Archive(archive);
*/

	uint16 nKids=0;
	for (HappyLink *p=child; p!=NULL; p=p->next)
		nKids++;
	archive->Write(&nKids,sizeof(uint16));

	for (HappyLink *p=child; p != NULL; p = p->next)
			p->Archive(archive);
	
	return B_OK;
}

HappyLink::HappyLink(BDataIO *archive)
{
	// numéro du lien
	uint32 l;
	archive->Read(&l,sizeof(uint32));
	link = l;
	
	// nom du lien
	uint16 s;
	archive->Read(&s,sizeof(uint16));
	char *tmp = new char[s+1];
	archive->Read(tmp,s);
	tmp[s]=0;
	name = tmp;
	delete tmp;
	
	// infos
	archive->Read(&s,sizeof(uint16));
	tmp = new char[s+1];
	archive->Read(tmp,s);
	tmp[s]=0;
	info = tmp;
	delete tmp;
	
/* //	autres HappyLink
	uint16 others;
	archive->Read(&others,sizeof(uint16));
	if (others & HLA_NEXT)
		next = new HappyLink(archive);
	else
		next = NULL;
	
	if (others & HLA_CHILD)
		child = new HappyLink(archive);
	else
		child = NULL;
*/
	next = NULL;
	uint16 nKids;
	archive->Read(&nKids,sizeof(uint16));
	
	if (nKids == 0)
		child = NULL;
	else
	{
		HappyLink **p = &child;
		for (unsigned int i=0; i<nKids; i++)
		{
			HappyLink *nl = new HappyLink(archive);
			*p = nl;
			p = &(nl->next);
		}
	}
}

unsigned int HappyLink::CountItems()
{
	unsigned int c1=0,c2=0;
	
	if (next!=NULL)
		c1 = next->CountItems();
	
	if (child!=NULL)
		c2 = child->CountItems();
		
	return c1+c2+1;
}

HappyLink *HappyLink::Search(const BString &nameToSearch, unsigned int generation, BList *list, bool onlyKids)
{
	if (generation == 0)
		return NULL;
		
	if (nameToSearch == name)
	{
		if (list == NULL)
			return this;
		else
			list->AddItem(this);
	}
	
	HappyLink *search;
	if ((!onlyKids) && (next != NULL) && ((search = next->Search(nameToSearch,generation,list,false)) != NULL))
		return search;
	
	if ((child != NULL) && ((search = child->Search(nameToSearch,generation-1,list,false)) != NULL))
		return search;
	
	return NULL;
}

bool HappyLink::Find(HappyLink **father, HappyLink **son, unsigned int linkNum, bool onlyKids)
{
	if (link==linkNum)
	{
		*son = this;
		return true;
	}
	
	if ((!onlyKids) && (next != NULL) && next->Find(father,son,linkNum,false))
		return true;
	
	if (child != NULL)
	{
		HappyLink *oldFather = *father;
		*father = this;
		
		if (child->Find(father,son,linkNum,false))
			return true;
		
		*father = oldFather;
	}
	return false;
}