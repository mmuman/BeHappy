## ********************************* ##
## ZETA Generic Makefile v3.0        ##

## Fill in this file to specify the project being created, and the referenced
## makefile-engine will do all of the hard work for you.

## Application Specific Settings ---------------------------------------------

# specify the name of the binary
NAME := BeHappy

# specify the type of binary
#	APP:	Application
#	SHARED:	Shared library or add-on
#	STATIC:	Static library archive
#	DRIVER: Kernel Driver
#	MODULE: Kernel Module
#	DECOR:	A window decorator project
TYPE := APP

#	add support for new Pe and Eddie features
#	to fill in generic makefile

#%{
# @src->@ 

#	specify the source files to use
#	full paths or paths relative to the makefile can be included
# 	all files, regardless of directory, will have their object
#	files created in the common object directory.
#	Note that this means this makefile will not work correctly
#	if two source files with the same name (source.c or source.cpp)
#	are included from different directories.  Also note that spaces
#	in folder names do not work well with this makefile.
SRCS := \
	AddOnPrefFile.cpp \
	BeHappy.cpp \
	BrowseView.cpp \
	Eraser.cpp \
	HTMLFile.cpp \
	HTMLProject.cpp \
	HappyLink.cpp \
	InfoWindow.cpp \
	LanguageHandler.cpp \
	Locale.cpp \
	MainWindow.cpp \
	Mover.cpp \
	NetView.cpp \
	Preferences.cpp \
	Tasks.cpp \
	TranslateEntities.cpp \
	Version.cpp \
	WindowMonitor.cpp \
	Add-ons/PathWindow.cpp

NOTUSED= \
	BeBookPathWin.cpp \
	HappyListView.cpp \

#	specify the resource files to use
#	full path or a relative path to the resource file can be used.
RSRCS := BeHappy.rsrc 
#Netpositive\ Replicant.rsrc

#	Specify your RDEF files, if any.
RDEFS := NetPositiveReplicant.rdef NetSurfReplicant.rdef

# @<-src@ 
#%}

#	end support for Pe and Eddie

#	specify additional libraries to link against
#	there are two acceptable forms of library specifications
#	-	if your library follows the naming pattern of:
#		libXXX.so or libXXX.a you can simply specify XXX
#		library: libbe.so entry: be
#		
#	- 	if your library does not follow the standard library
#		naming scheme you need to specify the path to the library
#		and it's name
#		library: my_lib.a entry: my_lib.a or path/my_lib.a
LIBS := be tracker

#	specify additional paths to directories following the standard
#	libXXX.so or libXXX.a naming scheme.  You can specify full paths
#	or paths relative to the makefile.  The paths included may not
#	be recursive, so include all of the paths where libraries can
#	be found.  Directories where source files are found are
#	automatically included.
LIBPATHS := 

#	additional paths to look for system headers
#	thes use the form: #include <header>
#	source file directories are NOT auto-included here
SYSTEM_INCLUDE_PATHS := Add-ons

#	additional paths to look for local headers
#	thes use the form: #include "header"
#	source file directories are automatically included
LOCAL_INCLUDE_PATHS := 

#	specify the level of optimization that you desire
#	NONE, SOME, FULL
OPTIMIZE	:=

#	specify any preprocessor symbols to be defined.  The symbols will not
#	have their values set automatically; you must supply the value (if any)
#	to use.  For example, setting DEFINES to "DEBUG=1" will cause the
#	compiler option "-DDEBUG=1" to be used.  Setting DEFINES to "DEBUG"
#	would pass "-DDEBUG" on the compiler's command line.
DEFINES		:= _NODEBUG _ZETA_TS_FIND_DIR_=1

#	specify special warning levels
#	if unspecified default warnings will be used
#	NONE = supress all warnings
#	ALL = enable all warnings
WARNINGS	:= 

#	specify whether image symbols will be created
#	so that stack crawls in the debugger are meaningful
#	if TRUE symbols will be created
SYMBOLS		:=

#	specify debug settings
#	if TRUE will allow application to be run from a source-level
#	debugger.  Note that this will disable all optimzation.
DEBUGGER	:= 

#	specify additional compiler flags for all files
COMPILER_FLAGS	:=

#	specify additional linker flags
LINKER_FLAGS	:=

#	specify the version of this particular item
#	(for example, -app 3 4 0 d 0 -short 340 -long "340 "`echo -n -e '\302\251'`"1999 GNU GPL") 
#E	This may also be specified in a resource.
APP_VERSION	:= 

#	(for TYPE == DRIVER only) Specify desired location of driver in the /dev
#	hierarchy. Used by the driverinstall rule. E.g., DRIVER_PATH = video/usb will
#	instruct the driverinstall rule to place a symlink to your driver's binary in
#	~/add-ons/kernel/drivers/dev/video/usb, so that your driver will appear at
#	/dev/video/usb when loaded. Default is "misc".
DRIVER_PATH	:= 

#	Specify if you want the object files to be somewhere besides the default location.
OBJ_DIR		:=

#	Specify a non default placement for the target
TARGET_DIR	:=

#	Specify a directory for the 'install' target.
INSTALL_DIR	:= /boot/apps/BeHappy
 
#	Specify the name of this makefile.
#	If you leave this blank, the makefile will not be considered as part of the
#	dependenies for the project, and the project will not be rebuilt when the makefile
#	is changed
MAKEFILE	:= 

#	Specify TRUE if you want the install target to create links in the BeMenu
MENU_LINKS	:= 

#	Related to MENU_LINKS, specify the name of the direcotry in the BeMenu
#	you wish the link to go in. If the directory does not exist, it will be
#	created.
APP_MENU	:= 

#       If, for some reason, you don't want to use the dependencies (flex and yacc seem to choke 
#       on them), set this to false 
DODEPS		:= 

#	Set this variable if you have an svg text file you wish to use as your targets
#	icon.
SVG_ICON	:=

#	If you have some fancy custom build steps to do, specify them here
EXTRA_BUILD_STEPS	=

#	If you have some other files that should trigger a re-link, such as libs in the same
#	project that may get rebuilt, specify the full path to them here.
EXTRA_DEPS :=

###########################################################################################
# The following variables are commented out here because the can be very useful to just 
# set at the command line or in the env at time of compiling, allowing you to leave your 
# makefile the same, but change the build types easily.


#	If you wish to have the program output a profiling session file which can be read by bprof,
#	set this to 'true'
#BUILD_PROFILE	:=

#	If you wish to have a debug build,
#	set this to 'true'
#BUILD_DEBUG	:=

#	If you wish to have a build which can do memory checking when MALLOC_DEBUG=15 is set,
#	set this to 'true'
#CHECK_MEM		:=

#	If you want to see the complete build line for every file, then set this to 'true',
#	otherwise it will tell you at the end what the build flags were.
#CHATTY			:=



## include the makefile-engine
include $(BUILDHOME)/etc/makefile-engine