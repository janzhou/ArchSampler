CXX=gcc 
INCLUDE_PATH = ${SST_ELEMENTS_HOME}/include/sst/elements
LIBS_PATH= ${SST_ELEMENTS_HOME}/lib/sst-elements-library
COPS = -g -O0
LIBS= -fopenmp -lpthread

all: sampling_test.exe benchmark.exe sort_checker.exe

%.exe: %.o pcm.o movie.o arielapi.o amazon_movies.o amazon_movies_trim.o keycnt.o
	$(CXX) -o $@ $^ -L$(LIBS_PATH) -I$(INCLUDE_PATH)  $(LIBS) $(COPS)

%.o: %.c pcm.h movie.h arielapi.h amazon_movies.h amazon_movies_trim.h keycnt.h
	$(CXX) -o $@ -c $< -L$(LIBS_PATH) -I$(INCLUDE_PATH)  $(LIBS) $(COPS)

clean:
	rm -f *.exe *.o
