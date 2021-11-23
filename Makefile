DEBUG=-fsanitize=address -fsanitize=undefined -fsanitize=leak -ggdb3 -fanalyzer
CFLAGS=-std=c18 -Wall -Wextra -Werror -pedantic $(DEBUG)
OBJ=bot.o irc.o lexer.o network.o string.o
CC=gcc

all: bot

%.o: %.c
	$(CC) $(CFLAGS) -c $^

bot: $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

clean:
	-rm *.o
	-rm *.gch
	-rm bot

.PHONY: all
