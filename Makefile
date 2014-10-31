CC := gcc
CFLAGS := -Wall -Wno-unused-function -Wno-unused-variable -DDEBUG -g
TARGET := lexer
SRC_FILES := $(wildcard src/*.c)
OBJ_FILES := $(patsubst %.c, %.o, $(SRC_FILES))
ifeq ($(OS), Windows_NT)
	TARGET := $(TARGET).exe
endif

all: $(OBJ_FILES)
	$(CC) $? -o $(TARGET)

src/%.o: src/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

# Tell make not associate `clean` with a file
.PHONY: clean, test

# test
test:
	./$(TARGET) lib/hello.test

# clean up
clean:
	rm -rf src/*.o
