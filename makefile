CC = gcc

CFLAGS = -Wall -Wextra -pedantic -O0 

CSRC = temp.c
VSRC = temp.v
TARGET = main

c: $(SRC)
	cat $(CSRC)
	$(CC) $(CFLAGS) $(CSRC) -o $(TARGET)

clear: $(TARGET)
	clear
	rm $(TARGET)
cpp: 
	cat $(VSRC)
	cmake --build cmake-build-debug
	./cmake-build-debug/v_lang
