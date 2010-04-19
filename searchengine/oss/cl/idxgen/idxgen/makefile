
OBJS = clucenetest.cpp \
       corpus.cpp \
       cpixstrtools.cpp \
       cpixfstools.cpp


CC = g++

DEFS =

INCLUDES = -I ../clucene-core-0.9.21/src

LIBS = ../clucene-core-0.9.21/src/.libs/libclucene.a

EXE = idxgenerator.exe

perfmetrics.exe:
	$(CC) $(DEFS) $(INCLUDES) $(OBJS) $(LIBS) -o $(EXE)


clean:
	rm $(EXE) *.*~ *~

