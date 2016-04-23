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
DOBJ=obj

MKDIR=mkdir
WC=cl
WNAME=/Fe
WOBJ=/c
WCLEAN=DEL

LCLEAN=rm -f
LC=gcc -pthread --static -g
LNAME=-o
LOBJ=-c
LLINK=-lm

####################################################################################################
#
# WINDOWS TARGETS
#
####################################################################################################

server-win : server_monitor.c win\utilities.c mem_management.obj filesystree.obj mapping_structure.obj \
	mapping.obj myfile.obj settings_parser.obj syncmapping.obj notifications_bucket.obj utilities.obj \
	daemon.obj thread.obj time_utilities.obj received_notification.obj signal_handler.obj \
	server_commons.obj
	$(WC) $(WNAME)"server" server_monitor.c mem_management.obj filesystree.obj daemon.obj thread.obj  \
		mapping_structure.obj mapping.obj myfile.obj settings_parser.obj syncmapping.obj \
		notifications_bucket.obj utilities.obj time_utilities.obj received_notification.obj \
		signal_handler.obj server_commons.obj

client-win : client.c settings_parser.obj params_parser.obj networking.obj thread.obj
	$(WC) $(WNAME)"client" client.c settings_parser.obj params_parser.obj networking.obj thread.obj
####################################################################################################
# platform indipendent
####################################################################################################

params_parser.obj : params_parser.c
	$(WC) $(WOBJ) params_parser.c

mapping_structure.obj : mapping_structure.c
	$(WC) $(WOBJ) mapping_structure.c

filesystree.obj : filesystree.c
	$(WC) $(WOBJ) filesystree.c

server_commons.obj : server_commons.c
	$(WC) $(WOBJ) server_commons.c

daemon.obj : daemon.c
	$(WC) $(WOBJ) daemon.c

received_notification.obj : received_notification.c
	$(WC) $(WOBJ) received_notification.c

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

signal_handler.obj : win\signal_handler.c
	$(WC) $(WOBJ) win\signal_handler.c

time_utilities.obj : win\time_utilities.c
	$(WC) $(WOBJ) win\time_utilities.c

utilities.obj :  win\utilities.c
	$(WC) $(WOBJ)  win\utilities.c

syncmapping.obj : "win\syncmapping.c"
	$(WC) $(WOBJ) win\syncmapping.c

thread.obj : "win\thread.c"
	$(WC) $(WOBJ) win\thread.c

####################################################################################################
#
# LINUX TARGETS
#
####################################################################################################

server-linux : server_monitor.c mem_management.o filesystree.o mapping_structure.o \
	mapping.o myfile.o settings_parser.o syncmapping.o notifications_bucket.o thread.o daemon.o \
	time_utilities.o received_notification.o signal_handler.o server_commons.o
	$(LC) $(LNAME)"server" server_monitor.c mem_management.o filesystree.o thread.o daemon.o \
		mapping_structure.o mapping.o myfile.o settings_parser.o syncmapping.o notifications_bucket.o \
		time_utilities.o received_notification.o signal_handler.o server_commons.o $(LLINK)

client-linux : client.c settings_parser.o params_parser.o networking.o thread.o
	$(LC) $(LNAME)"client" client.c settings_parser.o params_parser.o networking.o thread.o $(LLINK)
####################################################################################################
# platform indipendent
####################################################################################################

params_parser.o : params_parser.c
	$(LC) $(LOBJ) params_parser.c

mapping_structure.o : mapping_structure.c
	$(LC) $(LOBJ) mapping_structure.c

filesystree.o : filesystree.c
	$(LC) $(LOBJ) filesystree.c

daemon.o : daemon.c
	$(LC) $(LOBJ) daemon.c

server_commons.o : server_commons.c
	$(LC) $(LOBJ) server_commons.c

received_notification.o : received_notification.c
	$(LC) $(LOBJ) received_notification.c

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

time_utilities.o : linux/time_utilities.c
	$(LC) $(LOBJ) "linux/time_utilities.c"

signal_handler.o : linux/signal_handler.c
	$(LC) $(LOBJ) "linux/signal_handler.c"

thread.o : linux/thread.c
	$(LC) $(LOBJ) "linux/thread.c"

syncmapping.o : linux/syncmapping.c
	$(LC) $(LOBJ) "linux/syncmapping.c"
