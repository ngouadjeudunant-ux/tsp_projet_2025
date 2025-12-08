CC=gcc
CFLAGS=-Iinclude -lm
LDFLAGS=-lm

ifeq ($(DEBUG),no)
	CFLAGS += -O3 -DNDEBUG
	LDFLAGS +=
else
	CFLAGS += -g 
	LDFLAGS +=
endif

EXEC=tsp
SRC= $(wildcard src/*.c)
OBJ= $(SRC:.c=.o)

all:
ifeq ($(DEBUG),yes)
	@echo "Generating in debug mode"
else
	@echo "Generating in release mode"
endif
	@$(MAKE) $(EXEC)

$(EXEC): $(OBJ)
	$(ECHO)$(CC) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) -o $@ -c $< $(CFLAGS)
