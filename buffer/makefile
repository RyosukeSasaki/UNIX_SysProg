SRCS = main.c command.c buffer.c blockfunc.c
OBJS = $(SRCS:%.c=%.o)
DEPS = $(SRCS:%.c=%.d)
PROG = bufcache
CFLAGS := -std=c99

main: $(OBJS)
	gcc $(CFLAGS) $(OBJS) -o $(PROG)

.c.o:
	gcc $(CFLAGS) -MD -MP -c $<

clean:
	rm $(PROG) $(OBJS) $(DEPS)

-include $(DEPS)