// traducteur html
//
// prend une BString, et change toutes les entity en caractères unicode

#include <String.h>
#include "TranslateEntities.h"

// nombre d'entités
#define c_NumEntities 50

// tableau des entités (dans l'ordre alphabétique)
static const char *s_EntityNames[c_NumEntities] = {
	"Aacute", "Acirc", "AElig", "Agrave", "Auml", "Ccedil", "Eacute", "Ecirc", "Egrave", "Euml", 
	"Iacute", "Icirc", "Igrave", "Iuml", "Oacute", "Ocirc", "OElig", "Ograve", "Ouml", "Otilde", "Yacute",
	"Yuml", "aacute", "acirc", "aelig", "agrave", "amp", "auml", "ccedil", "eacute", "ecirc", "egrave",
	"euml", "gt", "iacute", "icirc", "igrave", "iuml", "lt", "nbsp", "ndash", "oacute", "ocirc", "oelig",
	"ograve", "ouml", "otilde", "yacute", "yuml" };

// tableau des caractères à mettre à la place
const char *s_EntityCharacters[c_NumEntities] = {
	"Á", "Â", "Æ", "À", "Ä", "Ç", "É", "Ê", "È", "Ë",
	"Í", "Î", "Ì", "Ï", "Ó", "Ô", "Œ", "Ò", "Ö", "Õ", "Ý",
	"Ÿ", "á", "â", "æ", "à", "&", "ä", "ç", "é", "ê", "è",
	"ë", ">", "í", "î", "ì", "ï", "<", " ", "-", "ó", "ô", "œ", 
	"ò", "ö", "õ", "ý", "ÿ" };

void TranslateEntities(BString &str)
{
	// pointeur vers un caractère '&'
	int l_Pointer=-1;
	
	while ((l_Pointer = str.FindFirst('&',l_Pointer+1)) >= 0)
	{
		// recherche du ';' de fin
		unsigned int l_Pointer2;
		
		l_Pointer2 = str.FindFirst(';',l_Pointer);
		
		if (l_Pointer2 < 0)
			return;
			// s'il n'y a plus de ';' dans la chaîne, on peut arréter de chercher des '&', c'est fini
		
		BString l_Entity(str.String()+l_Pointer+1,l_Pointer2-l_Pointer-1);
		
		// recherche de l'entité dans la liste déclarée ci-dessus
		// on procède par dichotomie pour trouver la chaîne
		unsigned int l_Start = 0;
		unsigned int l_End = c_NumEntities;
		unsigned int l_StringIndex;

		while(l_Start != l_End)
		{
			// on regarde ce qu'il y a au milieu
			l_StringIndex = (l_End+l_Start)/2;
			
			int l_Comp = l_Entity.Compare(s_EntityNames[l_StringIndex]);
		
			if (l_Comp == 0)
			{
				l_Entity.Prepend("&");
				l_Entity.Append(";");
				str.Replace(l_Entity.String(),s_EntityCharacters[l_StringIndex],1,l_Pointer);
				break;	
			}			
			if (l_Comp>0)
				// la chaîne à chercher est après
				l_Start = ++l_StringIndex;
			else
				// la chaîne à chercher est avant
				l_End = l_StringIndex;
		}		
	}
}
	