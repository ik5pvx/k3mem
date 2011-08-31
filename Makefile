TARGET = k3mem

all: $(TARGET)

OBJS = main.o \
       k3comms.o \
       erCommand.o

# use paranoia warnings/cflags.
CFLAGS_WARNS = \
	-Wall -Wformat=2 -Wshadow -Wmissing-prototypes \
	-Wstrict-prototypes -Wdeclaration-after-statement \
	-Wpointer-arith -Wwrite-strings -Wcast-align \
	-Wbad-function-cast -Wmissing-format-attribute \
	-Wformat-security -Wformat-nonliteral -Wno-long-long \
	-Wno-strict-aliasing -Wmissing-declarations

CFLAGS_EXTRAS = -O0 -ggdb3 -MMD

%.o: %.c
	$(CC) $(CFLAGS) $(CFLAGS_WARNS) $(CFLAGS_EXTRAS) -c -o $@ $<

$(TARGET): $(OBJS)
	$(CC) -o  $@ $^ $(LDFLAGS)

clean:
	@rm -f $(TARGET) $(OBJS) *.d

-include $(OBJS:.o=.d)
