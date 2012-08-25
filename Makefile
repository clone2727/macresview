all:
	g++ -Wall -g -c util.cpp -o util.o
	g++ -Wall -g -c macresfork.cpp -o macresfork.o
	g++ -Wall -g -c macresview.cpp -o macresview.o
	g++ -o macresview util.o macresfork.o macresview.o

clean:
	rm -f *.o
	rm -f macresview
