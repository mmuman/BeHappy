/* 
* BeHappy Test
*
* fichier main
*
*/

#ifndef HAPPYTEST_H
#define HAPPYTEST_H

#include <Application.h>

class HappyTest : public BApplication
{
public:
	HappyTest();
	~HappyTest();
	
	void MessageReceived(BMessage*);
};

#endif //HAPPYTEST_H