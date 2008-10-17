/*
*  BeHappy
*   by Sylvain Tertois
*
* add-on header
*
*
*/

#ifndef BHADD_ONS_H
#define BHADD_ONS_H

// browseview options
#define BW_SECONDLIST	0x01
#define BW_NEXTBUTTON	0x02
#define BW_SEARCHTXTV	0x04
#define BW_CONTAINS		0x08
#define BW_INFOTEXT		0x10


class HTMLProject;
class BMessage;

extern "C" __declspec(dllexport) HTMLProject *InstantiateProject(BMessage*);
extern "C" __declspec(dllexport) const char *projectName;
extern "C" __declspec(dllexport) const uint16 BHVersion;
extern "C" __declspec(dllexport) const uint16 BHVLastCompatible;

#define BHAO_VERSION 0x0104
#define BHAO_LCVERSION 0x0104

#endif //BHADD_ONS_H