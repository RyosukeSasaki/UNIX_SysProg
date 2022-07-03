SRCS = mydhcpserver.c
SRCC = mydhcpclient.c
OBJS = $(SRCS:%.c=%.o)
DEPS = $(SRCS:%.c=%.d)
OBJC = $(SRCC:%.c=%.o)
DEPC = $(SRCC:%.c=%.d)
CFLAGS := -std=gnu99

all: server client

server: $(OBJS)
	-@gcc $(CFLAGS) $(OBJS) -o mydhcpd

client: $(OBJC)
	-@gcc $(CFLAGS) $(OBJC) -o mydhcpc

.c.o:
	gcc $(CFLAGS) -MD -MP -c $<

clean: rmserv rmclient

rmserv:
	-@rm mydhcpd $(OBJS) $(DEPS)

rmclient:
	-@rm mydhcpc $(OBJC) $(DEPC)

verify: all clean

-include $(DEPS)