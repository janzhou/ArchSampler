CXX=g++ 

all: stream.exe multi_threads.exe

%.exe: %.o pcm.o arielapi.o
	$(CXX) -O3 -o $@ -fopenmp $^

%.o: %.c pcm.h arielapi.h
	$(CXX) -O3 -fopenmp -c $<

clean:
	rm -f *.exe *.o

