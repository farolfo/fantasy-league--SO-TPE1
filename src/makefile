.SILENT:

TARGET = all
OBJSCMN= ./ADT/ListADT.c ./ADT/HashMapADT.c ./misc.c
OBJSSERV= ./server/servstub.c ./server/server.c ./db/dbms.c
OBJSCL= ./client/client.c ./client/clstub.c ./ADT/TrieADT.c
OBJSIPC= ./transport/fifo.c ./transport/shmem.c ./transport/msgqueue.c ./transport/socket.c
CC = gcc
COPTS = -Wall -g
LD = $(CC)
DIR = ./bin/
LDOPTS = -o

$(TARGET):	$(OBJSSERV) $(OBJSCMN) $(OBJSIPC) $(OBJSCL)
	echo Compilando serverFifo...
	$(LD) $(LDOPTS) $(DIR)serverFifo $(OBJSSERV) $(OBJSCMN) ./transport/fifo.c -pthread $(COPTS)
	echo Compilando clientFifo...
	$(LD) $(LDOPTS) $(DIR)clientFifo $(OBJSCL) $(OBJSCMN) ./transport/fifo.c $(COPTS)
	echo Compilando serverShmem...
	$(LD) $(LDOPTS) $(DIR)serverShmem $(OBJSSERV) $(OBJSCMN) ./transport/shmem.c -pthread $(COPTS)
	echo Compilando clientShmem...
	$(LD) $(LDOPTS) $(DIR)clientShmem $(OBJSCL) $(OBJSCMN) ./transport/shmem.c $(COPTS)
	echo Compilando serverMsgq...
	$(LD) $(LDOPTS) $(DIR)serverMsgq $(OBJSSERV) $(OBJSCMN) ./transport/msgqueue.c -pthread $(COPTS)
	echo Compilando clientMsgq...
	$(LD) $(LDOPTS) $(DIR)clientMsgq $(OBJSCL) $(OBJSCMN) ./transport/msgqueue.c $(COPTS)
	echo Compilando serverSocket...
	$(LD) $(LDOPTS) $(DIR)serverSocket $(OBJSSERV) $(OBJSCMN) ./transport/socket.c -pthread $(COPTS)
	echo Compilando clientSocket...
	$(LD) $(LDOPTS) $(DIR)clientSocket $(OBJSCL) $(OBJSCMN) ./transport/socket.c $(COPTS)
	echo Done!


