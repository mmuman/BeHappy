/*
 * classe de localisation
 *
 * Le Fichier de localisation doit être un fichier texte de la forme suivante:
 *

#commentaires:
# les lignes qui commencent par un '#' sont ignorés
#
# ensuite on met le terme anglais entre crochets, puis les traductions dans
# les différentes langues en dessous
# la première traduction est particulière, elle contient les noms des langues

[English]
Français

# ensuite viennent les expressions à traduire.
# il doit toujours y avoir le même nombre de lignes après les crochets
# (le nombre de langues défini ci-dessus)
[About...]
A propos...

[Open]
Ouvrir

 *
 */

#ifndef LOCALE_H
#define LOCALE_H

#include <List.h>
#include <String.h>


class Locale
{
public:
	Locale(const char *fileName);
	~Locale();
	
	// Récupération de la liste des langues
	const BList *GetLanguagesList(bool invalidate = false);
		// la liste reste la propriété de l'objet Locale. Il ne faut pas la
		// modifier ou la supprimer
		// chaque objet de la BList est un const BString
		// si invalidate est true, on force une relecture du fichier
	
	void ChangeLanguage(const char *lang);
		// change la langue active
		// recharge le fichier dans tous les cas, même si on donne la langue qui a déjà
		// été sélectionnée

	const char *Translate(const char *toTranslate) const;
		// donne la traduction d'une chaîne
		// si aucune traduction n'est disponible, la chaîne
		// originale est retournée
	
private:
	BString m_FileName;
	BList m_LanguagesList;
	
	// données de traductions
	BList m_OriginalStrings;
	BList m_TranslatedStrings;
	
	// ce booléen indique si une lecture du fichier a déjà été faite
	bool m_FileRead;
	
	// fonctions de lecture du fichier
	/// pointeurs dans le fichier
	char *m_Data, *m_DataEnd, *m_Pointer;
	
	/// chargement des données
	void ReadFile();
	void FreeBuffer();
	inline bool EndOfFile() { return (m_Pointer >= m_DataEnd); }
	
	/// lecture d'une ligne
	void ReadLine(BString &line);	
	bool ReadString(BString &string,bool &isname);
			// rend true si on est à la fin du fichier
			// cette fonction fait comme la précédente, mais saute les commentaires
			// si la ligne est du type [nom], met "nom" dans string, et
			// met true dans isname. Sinon la chaîne est mise telle quelle dans
			// string, et isname est mis à false
	
	// libération des listes
	void EmptyLanguagesList();
	void EmptyStrings();

	// utilisée par GetLanguagesList et ChangeLanguage
	// charge le fichier, met à jour la liste des langues, et charge la première chaîne à
	// traduire (c-à-d la première chaîne entre '[' et ']' après les noms de langues)
	// retourne cette première chaîne. Le pointeur est mis juste après cette chaîne, donc
	// on peut conitnuer à lire les chaînes suivantes.
	// le fichier est chargé (ReadFile()) a été appelé, mais les buffers ne sont pas libérés. Donc:
	// * le fichier ne doit pas être ouvert avant cet appel
	// * on peut utiliser ReadLine et ReadString après cet appel
	// * il faut appeler FreeBuffer quand on en a fini
	BString *ReadBeginningOfFile();
	
	// utilisée par ChangeLanguage et Translate
	// cherche une chaîne dans les chaînes à traduire
	// rend true si la chaîne a été trouvée
	// met un index dans le paramètre stringIndex:
	// * si la chaîne a été trouvée, l'index de la chaîne
	// * si la chaîne n'a pas été trouvée, l'index de la chaîne qui suivrait dans l'ordre alphabétique
	bool LookForString(const char *string, unsigned int &stringIndex) const;
};

#endif