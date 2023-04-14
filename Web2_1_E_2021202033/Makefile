#######################################################################
## File Name : Makefile									 		  	 ##
## Date : 2023/04/11 												 ##
## Os : Ubuntu 16.04 LTS 64bits 									 ##
## Author : Sung Min Yoon									 		 ##
## Student ID : 2021202033 											 ##
## ----------------------------------------------------------------- ##
## Title : System Programming Assignment #1-3 				 		 ##
## Description : this file is Makefile for the Assignment #1-3 		 ##
#######################################################################

EXEC = html_ls
CC = gcc
SRCS = 2021202033_html_ls.c 
FLGS = -g -Wall -Werror -Wextra -fsanitize=address # to detect warning and debug

all : $(EXEC)

$(EXEC) : $(SRCS) # gcc 2021202033_simple_ls.c -o simple_ls
	    $(CC) $^ -o $@ 

clean : 
	rm -f $(EXEC)

debug : $(SRCS) 
	$(CC) $^ -o $(EXEC) $(FLGS)

.PHONY : all clean debug
