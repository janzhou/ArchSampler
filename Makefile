CXX=gcc 
INCLUDE_PATH = ${SST_ELEMENTS_HOME}/include/sst/elements
LIBS_PATH= ${SST_ELEMENTS_HOME}/lib/sst-elements-library
COPS = -g -O0
LIBS= -fopenmp -lpthread

all: thread_write.exe thread_read.exe movie_count.exe movie_count2.exe amazon_movies_count.exe sampling_test.exe benchmark.exe

%.exe: %.o pcm.o movie.o arielapi.o amazon_movies.o amazon_movies_trim.o
	$(CXX) -o $@ $^ -L$(LIBS_PATH) -I$(INCLUDE_PATH)  $(LIBS) $(COPS)

%.o: %.c pcm.h movie.h arielapi.h amazon_movies.h amazon_movies_trim.h
	$(CXX) -o $@ -c $< -L$(LIBS_PATH) -I$(INCLUDE_PATH)  $(LIBS) $(COPS)

clean:
	rm -f *.exe *.o
