/*
* BeHappy
*
* Parser
*
* fichier principal
*
*/

#include "Parser.h"
#include "ParserWindow.h"

Parser::Parser()	: BApplication("application/x.vnd-STertois.HappyParser")
{
	new ParserWindow;
}

int main()
{
	Parser myApp;
	myApp.Run();
}