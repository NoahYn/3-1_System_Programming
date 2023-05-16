#######################################################################
## File Name : Makefile									 		  	 ##
## Date : 2023/05/10 												 ##
## Os : Ubuntu 16.04 LTS 64bits 									 ##
## Author : Sung Min Yoon									 		 ##
## Student ID : 2021202033 											 ##
## ----------------------------------------------------------------- ##
## Title : System Programming Assignment #2-3				 		 ##
## Description : this file is Makefile for the Assignment #2-3 		 ##
#######################################################################

EXEC = adv_server
CC = gcc
SRCS = 2021202033_adv_server.c 
FLGS = -g -Wall -Werror -Wextra -fsanitize=address # to detect warning and debug

all : $(EXEC)

$(EXEC) : $(SRCS) 
	    $(CC) $^ -o $@ 

clean : 
	rm -f $(EXEC)

debug : $(SRCS) 
	$(CC) $^ -o $(EXEC) $(FLGS)

.PHONY : all clean debug
