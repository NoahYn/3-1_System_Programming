#######################################################################
## File Name : Makefile									 		  	 ##
## Date : 2023/05/30 												 ##
## Os : Ubuntu 16.04 LTS 64bits 									 ##
## Author : Sung Min Yoon									 		 ##
## Student ID : 2021202033 											 ##
## ----------------------------------------------------------------- ##
## Title : System Programming Assignment #3-3				 		 ##
## Description : this file is Makefile for the Assignment #3-3 		 ##
#######################################################################

EXEC = semaphore_server
CC = gcc
SRCS = 2021202033_semaphore_server.c 
FLGS = -g -fsanitize=address # to detect warning and debug

all : $(EXEC)

$(EXEC) : $(SRCS) 
	    $(CC) $^ -o $@ -lpthread

clean : 
	rm -f $(EXEC)

debug : $(SRCS) 
	$(CC) $^ -o $(EXEC) $(FLGS)

.PHONY : all clean debug
