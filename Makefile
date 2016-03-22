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
LC=gcc
LNAME=-o
LOBJ=-c
####################################################################################################
#
# WINDOWS TARGETS
#
####################################################################################################

server-win : server.c win\utilities.c mem_management.obj filesystree.obj common_utilities.obj mapping_structure.obj \
	mapping.obj myfile.obj settings_parser.obj syncmapping.obj notifications_bucket.obj
	$(WC) $(WNAME)"server" server.c mem_management.obj filesystree.obj common_utilities.obj \
		mapping_structure.obj mapping.obj myfile.obj settings_parser.obj syncmapping.obj notifications_bucket.obj
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

common_utilities.obj : common_utilities.c
	$(WC) $(WOBJ) common_utilities.c

mem_management.obj : mem_management.c
	$(WC) $(WOBJ) mem_management.c

settings_parser.obj : settings_parser.c
	$(WC) $(WOBJ) settings_parser.c


####################################################################################################
# platform dependent
####################################################################################################
mapping.obj : "win\mapping.c"
	$(WC) $(WOBJ) win\mapping.c

myfile.obj : "win\myfile.c" "win\utilities.c"
	$(WC) $(WOBJ) win\myfile.c win\utilities.c

syncmapping.obj : "win\syncmapping.c"
	$(WC) $(WOBJ) win\syncmapping.c


####################################################################################################
#
# LINUX TARGETS
#
####################################################################################################

server-linux : server.c mem_management.o filesystree.o common_utilities.o mapping_structure.o \
	mapping.o myfile.o settings_parser.o syncmapping.o notifications_bucket.o
	$(LC) $(LNAME)"server" server.c mem_management.o filesystree.o common_utilities.o \
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

common_utilities.o : common_utilities.c
	$(LC) $(LOBJ) common_utilities.c

mem_management.o : mem_management.c
	$(LC) $(LOBJ) mem_management.c

settings_parser.o : settings_parser.c
	$(LC) $(LOBJ) settings_parser.c

####################################################################################################
# platform dependent
####################################################################################################
mapping.o : linux/mapping.c include/mapping.h
	$(LC) $(LOBJ) "linux/mapping.c"

myfile.o : linux/myfile.c include/myfile.h
	$(LC) $(LOBJ) "linux/myfile.c"

syncmapping.o : linux/syncmapping.c include/syncmapping.h
	$(LC) $(LOBJ) "linux/syncmapping.c"
