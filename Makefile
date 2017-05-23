CXX=g++ 

stream.exe: stream.o pcm.o
	$(CXX) -O3 -o $@ -fopenmp $^

multi_threads.exe: multi_threads.o pcm.o
	$(CXX) -O3 -o $@ -fopenmp $^

%.o: %.c pcm.h
	$(CXX) -O3 -fopenmp -c $<

clean:
	rm *.exe *.o

