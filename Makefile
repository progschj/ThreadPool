all: example

example: example.cpp ThreadPool.h
	g++ -o $@ $< -std=c++11 -pthread

clean:
	rm example
