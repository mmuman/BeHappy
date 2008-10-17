/*
 * classe de localisation
 *
 */

#include  "Locale.h"
#include <File.h>
#include <ctype.h>

Locale::Locale(const char *p_FileName)
{
	m_FileName = p_FileName;
	m_FileRead = false;
	
	m_Data = m_DataEnd = m_Pointer = NULL;
}

Locale::~Locale()
{
	// il faut vider les listes
	EmptyLanguagesList();
	EmptyStrings();
	
	// et on libère le buffer, s'il est encore ouvert
	void FreeBuffer();
}

// trucs de bas niveau
/// vidage des listes
void Locale::EmptyLanguagesList()
{
	unsigned int l_nItems = m_LanguagesList.CountItems();
	for (unsigned int i=0; i<l_nItems; i++)
		delete ((BString*)m_LanguagesList.ItemAt(i));

	m_LanguagesList.MakeEmpty();
}

void Locale::EmptyStrings()
{
	/// phrases en anglais
	unsigned int l_nItems = m_OriginalStrings.CountItems();
	for (unsigned int i=0; i<l_nItems; i++)
		delete ((BString*)m_OriginalStrings.ItemAt(i));
	m_OriginalStrings.MakeEmpty();

	/// phrases traduites
	l_nItems = m_TranslatedStrings.CountItems();
	for (unsigned int i=0; i<l_nItems; i++)
		delete ((BString*)m_TranslatedStrings.ItemAt(i));
	m_TranslatedStrings.MakeEmpty();
}

/// chargement du fichier et libération de la mémoire
void Locale::ReadFile()
{
	// ouverture du fichier
	BFile l_File(m_FileName.String(),B_READ_ONLY);
	
	// allocation du buffer
	off_t l_FileSize;
	if (l_File.GetSize(&l_FileSize) != B_OK)
		return;
	
	if ((m_Data = new char[l_FileSize]) == NULL)
		return;
	
	// chargement des données
	l_File.Read(m_Data,l_FileSize);
	m_Pointer = m_Data;
	m_DataEnd = m_Data+l_FileSize;
}

void Locale::FreeBuffer()
{
	delete m_Data;
	m_Data = m_DataEnd = m_Pointer = NULL;
}

/// chargement d'une chaîne de caractères
void Locale::ReadLine(BString &line)
{
	// si on est déjà à la fin du fichier, on ne fait rien, à part rendre une chaîne vide
	if (EndOfFile())
	{
		line.SetTo("");
		return;
	}
	
	char *l_Pointer = m_Pointer;
	
	// recherche de la fin de la ligne
	while((l_Pointer<m_DataEnd) && (*l_Pointer != 0x0a))
		l_Pointer++;
	
	// on remplit l'objet BString
	line.SetTo(m_Pointer,l_Pointer-m_Pointer);
	
	// et on met à jour le pointeur sur le premier caractère de la ligne suivante
	m_Pointer = l_Pointer+1;
}

/// chargement d'une chaîne intéressante
bool Locale::ReadString(BString &string, bool &isname)
{
	// on boucle tant qu'on a pas trouvé ce que l'on cherche
	for(;;)
	{
		// si on est à la fin du fichier, c'est fini
		if (EndOfFile())
		{
			string.SetTo("");
			isname = false;
			
			return true;
		}
		// sinon, on lit une ligne
		ReadLine(string);

		// suppression des espaces devant et derrière
		/// devant
		while((string.Length() > 0) && isspace(string[0]))
			string.Remove(0,1);

		//// petite pause... si le premier caractère est un '#', autant arréter tout de suite
		//// pareil si la chaine est vide
		if ((string.Length() == 0) || (string[0] == '#'))
			continue;

		/// derrière
		{
			unsigned int l_Length = string.Length();
			while(isspace(string[--l_Length]))
				string.Truncate(l_Length);
		}

		// bon... est-ce un nom entre crochets?
		if (string[0] == '[')
		{
			// s'il n'y a pas le ] de fin, on déclenche une erreur
			if (string[string.Length()-1] != ']')
			{
				// erreur
			}
			
			// extraction du nom
			string.Remove(0,1);
			string.Truncate(string.Length()-1);
			
			isname = true;
			return false;	// ce n'est pas la fin du fichier
		}
		
		// non, c'est une chaîne normale
		isname=false;
		return false;	// ce n'est pas la fin du fichier 
	}
}

// vraies fonctions
const BList *Locale::GetLanguagesList(bool invalidate)
{
	// doit-on relire le fichier?
	if (!m_FileRead || invalidate)
	{
		// oui...
		delete ReadBeginningOfFile();
		
		// fini...
		FreeBuffer();
		m_FileRead = true;
	}
	
	return &m_LanguagesList;
}	

// lecture du début du fichier
BString *Locale::ReadBeginningOfFile()
{
	// on commence par vider la liste actuelle des langues
	EmptyLanguagesList();
		
	// lecture du fichier
	ReadFile();
	
	BString l_Line;
	bool l_IsName;

	if (ReadString(l_Line,l_IsName))
		// le fichier est vide
		return NULL;
	
	// si la première chaîne n'est pas un nom, erreur!
	if (!l_IsName)
	{
		// erreur
	}
	
	// boucle... on met tout dans la liste, jusqu'à ce que l'on trouve une autre chaîne 
	// entre '[' et ']'
	do
		m_LanguagesList.AddItem(new BString(l_Line));
	while (!ReadString(l_Line,l_IsName) && !l_IsName);
	
	return new BString(l_Line);
}

void Locale::ChangeLanguage(const char *lang)
{
	// on vide les listes de traduction
	EmptyStrings();
	
	// on commence par ouvrir le fichier et charger les langues
	BString l_Line;
	bool l_IsName;
	{
		BString *l_PointLine = ReadBeginningOfFile();
		if (l_PointLine == NULL)
			return; 	// le fichier est vide
			
		l_Line = *l_PointLine;
		delete l_PointLine;
	}
	
	// recherche de la langue voulue
	unsigned int l_NumLanguages = m_LanguagesList.CountItems();
		// nombre total de langues
	unsigned int l_LanguageNum = l_NumLanguages;
		// numéro de la langue que l'on va utiliser
	
	for (unsigned int i=0; i<l_NumLanguages; i++)
	{
		if ( *((BString*)m_LanguagesList.ItemAt(i)) == lang )
		{
			// trouvé
			l_LanguageNum = i;
			break;
		}
	}
	
	// si la langue n'a pas été trouvée, on sort (avec les listes vides)
	if (l_LanguageNum == l_NumLanguages)
		return;
	
	// remplissage des listes de traductions
	for (;;)
	{
		// on stoque la chaîne à traduire
		/// d'abord on cherche à quel endroit
		unsigned int l_Index;
		LookForString(l_Line.String(),l_Index);
		m_OriginalStrings.AddItem(new BString(l_Line), l_Index);
		
		// et on cherche la traduction
		for (unsigned int i=0; i<l_LanguageNum; i++)
			ReadString(l_Line,l_IsName);
		
		// on stoque la traduction
		m_TranslatedStrings.AddItem(new BString(l_Line), l_Index);
		
		// et on va à la chaîne suivante à traduire
		for (unsigned int i=0; i<l_NumLanguages-l_LanguageNum; i++)
		{
			// en lisant la chaîne, on regarde si on n'est pas à la fin du fichier
			if (ReadString(l_Line,l_IsName))
				break;
		}
	
		// la chaîne est-elle bien un 'nom'? (= avec des crochets)
		if (!l_IsName)
		{
			// erreur
			break;
		}
	}
	
	// voilà voilà... c'est chargé	
	FreeBuffer();		
}

bool Locale::LookForString(const char *string, unsigned int &stringIndex) const
{
	stringIndex = 0;
	
	// on procède par dichotomie pour trouver la chaîne
	unsigned int l_Start = 0;
	unsigned int l_End = m_OriginalStrings.CountItems();

	while(l_Start != l_End)
	{
		// on regarde ce qu'il y a au milieu
		stringIndex = (l_End+l_Start)/2;
		
		int l_Comp = ((BString*)m_OriginalStrings.ItemAt(stringIndex))->ICompare(string);
		
		if (l_Comp == 0)
			return true;	// trouvé!!
		
		if (l_Comp<0)
			// la chaîne à chercher est après
			l_Start = ++stringIndex;
		else
			// la chaîne à chercher est avant
			l_End = stringIndex;
	}
	
	// on ne l'a pas trouvé... on se trouve juste après
	return false;	
}

const char *Locale::Translate(const char *toTranslate) const
{
	// on cherche la chaîne à traduire
	unsigned int l_Index;
	if (LookForString(toTranslate, l_Index))
		// trouvée!
		return ((BString*)m_TranslatedStrings.ItemAt(l_Index))->String();
	
	// pas trouvée... on rend la chaîne d'origine
	return toTranslate;
}