/*
* BeHappy
*
*
* Gestion des préférences liées à chaque add-on
*
*/

class BMessage;
class BPath;
#include <String.h>

class AddOnPrefFile
{
public:
	AddOnPrefFile(const char *name);
	~AddOnPrefFile();
	
	BMessage *GetIndex();
	BMessage *GetPrefs();
	
	void SetIndex(const BMessage*);
	void SetPrefs(const BMessage*);

private:
	BMessage *archive;
	BString name;
};