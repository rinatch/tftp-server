#Makefile for creating the 'ttftps' program

CC=gcc
CFLAGS=-g -Wall
CCLINK=$(CC)
LIBS=
OBJS=main.o server_tftp.o
RM=rm -f

#Creating the executable(ttftps)
ttftps:$(OBJS)
	$(CCLINK) -o ttftps $(OBJS) $(LIBS)

#creating the object file using default rules 
main.o: main.c server_tftp.h
server_tftp.o: server_tftp.c server_tftp.h

#Cleaning old files before new make
clean:
	$(RM) ttftps tftp_server_test *.o *.bak *~"#"*core
