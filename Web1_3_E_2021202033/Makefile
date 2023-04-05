#######################################################################
## File Name : Makefile									 		  	 ##
## Date : 2023/04/02 												 ##
## Os : Ubuntu 16.04 LTS 64bits 									 ##
## Author : Sung Min Yoon									 		 ##
## Student ID : 2021202033 											 ##
## ----------------------------------------------------------------- ##
## Title : System Programming Assignment #1-2 				 		 ##
## Description : this file is Makefile for the Assignment #1-2 		 ##
#######################################################################

EXEC = spls_final
CC = gcc
SRCS = 2021202033_final_ls.c 
FLGS = -g -Wall -Werror -Wextra # to detect warning and debug

all : $(EXEC)

$(EXEC) : $(SRCS) # gcc 2021202033_simple_ls.c -o simple_ls
	    $(CC) $^ -o $@ 

clean : 
	rm -f $(EXEC)

debug : $(SRCS) 
	$(CC) $^ -o $(EXEC) $(FLGS)

.PHONY : all clean debug
