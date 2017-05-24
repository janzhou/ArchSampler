CXX=g++
OUT=pcm
INCLUDE_PATH = ${HOME}/local/sstelements-7.0.0/include/sst/elements/ariel
LIBS_PATH= ${HOME}/local/sstelements-7.0.0/lib/sst-elements-library
LIBS= -lpthread -larielapi

all: pcm

pcm: pcm.c pcm.h app.c app.h multi_threads.c
	$(CXX) -O0 -o $(OUT) pcm.c app.c multi_threads.c\
	 -L$(LIBS_PATH) $(LIBS)\
	  -I$(INCLUDE_PATH)

clean:
	rm -f $(OUT)

