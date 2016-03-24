# Makefile
# Author: Cristian Di Pietrantonio
# 2016
#
# This makefile contains all the needed rules to compile executables for both windows and linux.
# Rules are divided in two sections, Windows and linux
# WINDOWS TARGETS: server-win, client-win
# LINUX TARGETS: server-linux, client-linux


####################################################################################################
#
# VARIABLES
#
####################################################################################################
WC=cl
WNAME=/Fe
WOBJ=/c
WCLEAN=DEL

LCLEAN=rm -f
LC=gcc --static -g
LNAME=-o
LOBJ=-c
####################################################################################################
#
# WINDOWS TARGETS
#
####################################################################################################

server-win : server.c win\utilities.c mem_management.obj filesystree.obj mapping_structure.obj \
	mapping.obj myfile.obj settings_parser.obj syncmapping.obj notifications_bucket.obj utilities.obj
	$(WC) $(WNAME)"server" server.c mem_management.obj filesystree.obj  \
		mapping_structure.obj mapping.obj myfile.obj settings_parser.obj syncmapping.obj notifications_bucket.obj utilities.obj
#	 $(WCLEAN) *.obj

####################################################################################################
# platform indipendent
####################################################################################################

mapping_structure.obj : mapping_structure.c
	$(WC) $(WOBJ) mapping_structure.c

filesystree.obj : filesystree.c
	$(WC) $(WOBJ) filesystree.c

notifications_bucket.obj : notifications_bucket.c
	$(WC) $(WOBJ) notifications_bucket.c

mem_management.obj : mem_management.c
	$(WC) $(WOBJ) mem_management.c

settings_parser.obj : settings_parser.c
	$(WC) $(WOBJ) settings_parser.c


####################################################################################################
# platform dependent
####################################################################################################
mapping.obj : "win\mapping.c"
	$(WC) $(WOBJ) win\mapping.c

myfile.obj : "win\myfile.c"
	$(WC) $(WOBJ) win\myfile.c

utilities.obj :  win\utilities.c
	$(WC) $(WOBJ)  win\utilities.c

syncmapping.obj : "win\syncmapping.c"
	$(WC) $(WOBJ) win\syncmapping.c


####################################################################################################
#
# LINUX TARGETS
#
####################################################################################################

server-linux : server.c mem_management.o filesystree.o mapping_structure.o \
	mapping.o myfile.o settings_parser.o syncmapping.o notifications_bucket.o
	$(LC) $(LNAME)"server" server.c mem_management.o filesystree.o  \
		mapping_structure.o mapping.o myfile.o settings_parser.o syncmapping.o notifications_bucket.o
	$(LCLEAN) *.o
####################################################################################################
# platform indipendent
####################################################################################################

mapping_structure.o : mapping_structure.c
	$(LC) $(LOBJ) mapping_structure.c

filesystree.o : filesystree.c
	$(LC) $(LOBJ) filesystree.c

notifications_bucket.o : notifications_bucket.c
	$(LC) $(LOBJ) notifications_bucket.c


mem_management.o : mem_management.c
	$(LC) $(LOBJ) mem_management.c

settings_parser.o : settings_parser.c
	$(LC) $(LOBJ) settings_parser.c

####################################################################################################
# platform dependent
####################################################################################################
mapping.o : linux/mapping.c
	$(LC) $(LOBJ) "linux/mapping.c"

myfile.o : linux/myfile.c
	$(LC) $(LOBJ) "linux/myfile.c"

syncmapping.o : linux/syncmapping.c
	$(LC) $(LOBJ) "linux/syncmapping.c"
