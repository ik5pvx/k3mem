OBJS = main.o k3comms.o erCommand.o
TARGET = k3mem

$(TARGET): $(OBJS)
	gcc -o $(TARGET) $(OBJS)

#main.o: erCommand.h
#erCommand.o: erCommand.h

clean:
	-rm $(TARGET) $(OBJS)

#erCommand.h: erCommand.c
#	cextract -E +s $< > $@

#k3comms.h: k3comms.c
#	cextract -E +s $< > $@

