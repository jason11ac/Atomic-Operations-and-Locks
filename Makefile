#Compiling with gcc
CC = gcc

#-g is for debugging purposes (gdb)
#-pthread is because the program uses threads
CFLAGS = -g -pthread

TARGET = lab2c
TARGET2 = SortedList

all: #Make the executable
	$(CC) $(CFLAGS) $(TARGET).c $(TARGET2).c -o $(TARGET)


clean: $(TARGET)  #make clean
	rm $(TARGET)

dist: $(TARGET)  #make dist
	tar -cvzf lab2c-504487052.tar.gz $(TARGET2).h $(TARGET2).c $(TARGET).c Makefile 2C_Graph1.png README
