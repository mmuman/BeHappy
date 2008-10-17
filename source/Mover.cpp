/*
*       Be Happy
*
* vue qui sert à redimensionner d'autres vues
*/

#include "Mover.h"
#include <Application.h>
#include <stdio.h>	// à virer

static const char verticalMouse[] =
{ 16,1,7,7,
  0x0,0x0,0x01,0x80,0x01,0x80,0x01,0x80,
  0x01,0x80,0x11,0x88,0x31,0x8C,0x7D,0xBE,
  0x7D,0xBE,0x31,0x8C,0x11,0x88,0x01,0x80,
  0x01,0x80,0x01,0x80,0x01,0x80,0x0,0x0,

  0x03,0xC0,0x03,0xC0,0x03,0xC0,0x03,0xC0,
  0x3B,0xDC,0x7B,0xDE,0xFF,0xFF,0xFF,0xFF,
  0xFF,0xFF,0xFF,0xFF,0x7B,0xDE,0x3B,0xDC,
  0x03,0xC0,0x03,0xC0,0x03,0xC0,0x03,0xC0 };

static const char horizontalMouse[] =
{ 16,1,7,7,
  0x00,0x00,0x01,0x80,0x03,0xC0,0x07,0xE0,
  0x01,0x80,0x01,0x80,0x00,0x00,0x7F,0xFE,
  0x7F,0xFE,0x00,0x00,0x01,0x80,0x01,0x80,
  0x07,0xE0,0x03,0xC0,0x01,0X80,0x00,0x00,
 
  0x03,0xC0,0x07,0xE0,0x0F,0xF0,0x0F,0xF0,
  0x0F,0xF0,0x02,0xC0,0xFF,0xFF,0xFF,0xFF,
  0xFF,0xFF,0xFF,0xFF,0x03,0xC0,0x0F,0xF0,
  0x0F,0xF0,0x0F,0xF0,0x07,0xE0,0x03,0xC0 };
   
Mover::Mover(BRect r,uint32 rm,bool v)
	: BView(r,NULL,rm,B_WILL_DRAW)
{
	vertical = v;
	moving = false;
}

Mover::~Mover()
{
}

void Mover::Draw(BRect rect)
{
/*	BRect r = Bounds();
	r.top+=1;
	r.left+=1;
	r.right-=1;
	r.bottom-=1;
	
	SetHighColor(192,192,192);
	FillRect(r);
	
	r=Bounds();
	SetHighColor(255,255,255);
	StrokeLine(r.LeftBottom(),r.LeftTop());
	StrokeLine(r.LeftTop(),r.RightTop());
	
	SetHighColor(168,168,168);
	StrokeLine(r.RightTop(),r.RightBottom());
	StrokeLine(r.RightBottom(),r.LeftBottom());
	
	SetHighColor(208,208,208);
	StrokeLine(r.LeftBottom(),r.LeftBottom());
	StrokeLine(r.RightTop(),r.RightTop());
*/
	BRect r = Bounds();
	SetHighColor(152,152,152);
	if (vertical)
	{
		StrokeLine(r.LeftTop(),r.LeftBottom());
		StrokeLine(r.RightTop()+BPoint(-1,0),r.RightBottom()+BPoint(-1,0));
		StrokeLine(r.RightTop(),r.RightBottom());
	}
	else
	{
		StrokeLine(r.LeftTop(),r.RightTop());
		StrokeLine(r.LeftBottom()+BPoint(0,-1),r.RightBottom()+BPoint(0,-1));
		StrokeLine(r.LeftBottom(),r.RightBottom());
	}
	
	SetHighColor(255,255,255);
	if (vertical)
		StrokeLine(r.LeftTop()+BPoint(1,0),r.LeftBottom()+BPoint(1,0));
	else
		StrokeLine(r.LeftTop()+BPoint(0,1),r.RightTop()+BPoint(0,1));
	
	SetHighColor(232,232,232);
	if (vertical)
		StrokeLine(r.RightTop()+BPoint(-2,0),r.RightBottom()+BPoint(-2,0));
	else
		StrokeLine(r.LeftBottom()+BPoint(0,-2),r.RightBottom()+BPoint(0,-2));
}

void Mover::MouseUp(BPoint p)
{
	moving = false;
}

void Mover::MouseDown(BPoint p)
{
	clickPoint = p;
	moving = true;
	
	// calcul des limites de déplacement du curseur
	limits = Parent()->ConvertToScreen(Parent()->Bounds());
	limits.OffsetBy(-clickPoint.x,-clickPoint.y);
	limits.InsetBy(75,50);
	limits.left += 50;
	limits.right -= 50;
	SetMouseEventMask(B_POINTER_EVENTS,B_NO_POINTER_HISTORY);
}

void Mover::MouseMoved(BPoint p,uint32 transit,const BMessage *bla)
{
	if (moving)
	{
		uint32 dummy;
		GetMouse(&p,&dummy,false);
		p = ConvertToScreen(p);
		BMessage moved(MOVER_MESSAGE);
		moved.AddPointer(MM_VIEW,this);
		
		float coord;
		if (vertical)
			coord = p.x-clickPoint.x;
		else
			coord = p.y-clickPoint.y;
		moved.AddFloat(MM_COORD,coord);
				
		// est-on dans les limites?
		bool l_InLimits = true;
		if (!vertical)
		{
			if ( (coord < limits.top) || (coord > limits.bottom))
				l_InLimits = false;
		}
		else
		{
			if ( (coord < limits.left) || (coord > limits.right))
				l_InLimits = false;
		}
		
		if (l_InLimits)		
			Window()->PostMessage(&moved);
	}
	else
		switch(transit)
		{
			case B_ENTERED_VIEW:
				if(vertical)
					be_app->SetCursor(verticalMouse);
				else
					be_app->SetCursor(horizontalMouse);
			break;
			
			case B_EXITED_VIEW:	
			case B_OUTSIDE_VIEW:
				be_app->SetCursor(B_HAND_CURSOR);
			break;
			
			default:
			break;
		}
}
