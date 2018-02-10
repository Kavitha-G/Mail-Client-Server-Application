CC=gcc
all: emailserver emailclient
emailserver:emailserver.c
	$(CC) $^ -o $@
emailclient:emailclient.c 
	$(CC) $^ -o $@