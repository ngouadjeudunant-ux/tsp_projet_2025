CC=gcc
FLAGS=-Iinclude -lm

ifeq ($(DEBUG),no)
	FLAGS += -O3 -DNDEBUG
else
	FLAGS += -g 
endif

EXEC=tsp
SRC= $(wildcard src/*.c)
OBJ = $(patsubst src/%.c, build/%.o, $(SRC))

all:
ifeq ($(DEBUG),yes)
	@echo "Generating in debug mode"
else
	@echo "Generating in release mode"
endif
	@$(MAKE) bin/$(EXEC)

bin/$(EXEC): $(OBJ)
	mkdir -p bin
	$(CC) -o $@ $^ $(FLAGS)

build/%.o: src/%.c
	mkdir -p build
	$(CC) -o $@ -c $< $(FLAGS)

clean:
	rm -rf build/*.o