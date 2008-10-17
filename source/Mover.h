/*
*       Be Happy
*
* vue qui sert à redimensionner d'autres vues
*/

/* on définit à la construction si la barre est verticale ou horizontale
*  la fenêtre reçoit un message avec le déplacement à effectuer
*  what = MOVER_MESSAGE
*  MM_COORD (float) nouvelle coordonnée (dans le repère de l'écran)
*  MM_VIEW (pointer) pointeur vers la vue qui a envoyé le message
*/

#ifndef MOVER_H
#define MOVER_H

#define MOVER_MESSAGE 'MVMO'
#define MM_COORD "coord"
#define MM_VIEW "view"

#include <InterfaceKit.h>

class Mover : public BView
{
public:
	Mover(BRect r,uint32 resizingMode,bool vertical);
	~Mover();
	
	virtual void Draw(BRect);
	virtual void MouseUp(BPoint);
	virtual void MouseDown(BPoint);
	virtual void MouseMoved(BPoint,uint32,const BMessage*);

private:
	bool vertical;
	bool moving;
	BPoint clickPoint;
	BRect limits;
};

#endif //MOVER_H