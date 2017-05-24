CXX=g++ 

all: stream.exe multi_threads.exe

%.exe: %.o pcm.o
	$(CXX) -O3 -o $@ -fopenmp $^

%.o: %.c pcm.h
	$(CXX) -O3 -fopenmp -c $<

clean:
	rm *.exe *.o

