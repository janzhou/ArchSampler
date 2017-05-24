CXX=g++ 
INCLUDE_PATH = ${SST_ELEMENTS_HOME}/include/sst/elements/ariel
LIBS_PATH= ${SST_ELEMENTS_HOME}/sstelements-7.0.0/lib/sst-elements-library
#LIBS= -lpthread -fopenmp
LIBS= -fopenmp

all: stream.exe multi_threads.exe

%.exe: %.o pcm.o arielapi.o app.o
	$(CXX) $(LIBS) -L$(LIBS_PATH) -I$(INCLUDE_PATH) -o $@ $^

%.o: %.c pcm.h arielapi.h app.h
	$(CXX) $(LIBS) -L$(LIBS_PATH) -I$(INCLUDE_PATH) -o $@ -c $<

clean:
	rm -f *.exe *.o

