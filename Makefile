CC=g++
CPPFLAGS=-I.
CFLAGS=-W -Wall -Wextra -fsanitize=address
PROG= game
CSRC= game.cpp
HSRC= 
OBJ= $(CSRC:.c=.o)

all: $(PROG)

$(PROG): $(OBJ)
	$(CC) $^ -o $@

%.o: %.c %.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $<

clean:
	@rm -f *.o *~ $(PROG)
