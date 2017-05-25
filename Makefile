CXX=g++ 
INCLUDE_PATH = ${SST_ELEMENTS_HOME}/include/sst/elements/ariel
LIBS_PATH= ${SST_ELEMENTS_HOME}/lib/sst-elements-library
LIBS= -fopenmp -O3

all: stream.exe thread_write.exe movie_count.exe

%.exe: %.o pcm.o movie.o arielapi.o
	$(CXX) $(LIBS) -L$(LIBS_PATH) -I$(INCLUDE_PATH) -o $@ $^

%.o: %.c pcm.h movie.h arielapi.h
	$(CXX) $(LIBS) -L$(LIBS_PATH) -I$(INCLUDE_PATH) -o $@ -c $<

clean:
	rm -f *.exe *.o

