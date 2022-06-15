SRCS = main.c parseline.c execline.c
OBJS = $(SRCS:%.c=%.o)
DEPS = $(SRCS:%.c=%.d)
PROG = mysh
CFLAGS := -std=gnu99

main: $(OBJS)
	gcc $(CFLAGS) $(OBJS) -o $(PROG)

.c.o:
	gcc $(CFLAGS) -MD -MP -c $<

clean:
	rm $(PROG) $(OBJS) $(DEPS)

test: main clean

-include $(DEPS)