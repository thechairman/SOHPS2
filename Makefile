CC=g++ -g

all: minlogic

minlogic: minlogic.cpp
	$(CC) -o minlogic $<

