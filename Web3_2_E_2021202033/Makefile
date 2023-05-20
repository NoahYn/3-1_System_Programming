#######################################################################
## File Name : Makefile									 		  	 ##
## Date : 2023/05/20 												 ##
## Os : Ubuntu 16.04 LTS 64bits 									 ##
## Author : Sung Min Yoon									 		 ##
## Student ID : 2021202033 											 ##
## ----------------------------------------------------------------- ##
## Title : System Programming Assignment #3-2				 		 ##
## Description : this file is Makefile for the Assignment #3-2 		 ##
#######################################################################

EXEC = ipc_server
CC = gcc
SRCS = 2021202033_ipc_server.c 
FLGS = -g -Wall -Werror -Wextra -fsanitize=address # to detect warning and debug

all : $(EXEC)

$(EXEC) : $(SRCS) 
	    $(CC) $^ -o $@ -lpthread

clean : 
	rm -f $(EXEC)

debug : $(SRCS) 
	$(CC) $^ -o $(EXEC) $(FLGS)

.PHONY : all clean debug
