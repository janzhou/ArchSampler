CXX=gcc 
INCLUDE_PATH = ${SST_ELEMENTS_HOME}/include/sst/elements
LIBS_PATH= ${SST_ELEMENTS_HOME}/lib/sst-elements-library
LIBS= -fopenmp -lpthread -O0 

all: thread_write.exe thread_read.exe movie_count.exe movie_count2.exe

%.exe: %.o pcm.o movie.o arielapi.o
	$(CXX) -o $@ $^ -L$(LIBS_PATH) -I$(INCLUDE_PATH)  $(LIBS)

%.o: %.c pcm.h movie.h arielapi.h
	$(CXX) -o $@ -c $< -L$(LIBS_PATH) -I$(INCLUDE_PATH)  $(LIBS)

clean:
	rm -f *.exe *.o

