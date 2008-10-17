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

// Vue Netpositive

#ifndef NETVIEW_H
#define NETVIEW_H

#include <View.h>

class NetView : public BView
{
public:
	NetView(BRect frame, const char *url);
	~NetView();
	
	void NewUrl(const char *path); // on donne le chemin complet vers le fichier
	void NewUrlFull(const char *url);	// on donne l'url en entier (avec file:// ou http://)
	

private:
	BTextControl *urlCont;		// TexControl pour faire marcher BeHappy sous la R4.5
};

#endif