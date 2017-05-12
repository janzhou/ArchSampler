CXX=g++ 

stream.exe: stream.o
	$(CXX) -O3 -o stream.exe -fopenmp stream.o

stream.o: stream.c
	$(CXX) -O3 -o stream.o -fopenmp -c stream.c

clean:
	rm stream stream.o

