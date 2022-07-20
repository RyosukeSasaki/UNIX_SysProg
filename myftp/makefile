SRCS = myftpd.c common.c
SRCC = myftpc.c common.c
EXES = myftpd
EXEC = myftpc
CFLAGS := -std=gnu99

all: server client

server: $(SRCS)
	-@gcc $(CFLAGS) $(SRCS) -o $(EXES)

client: $(SRCC)
	-@gcc $(CFLAGS) $(SRCC) -o $(EXEC)

clean: rmserv rmclient

rmserv:
	-@rm $(EXES)

rmclient:
	-@rm $(EXEC)

verify: all clean

-include $(DEPS)