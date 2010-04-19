# MAKEFILE BASED ON THE ORIGINAL GNUmakefile, but tailored for non-GNU
# make MS-DOS batch command line environment, etc.
#

#CC = c:\Program Files\Nokia\Carbide.c++ 1.3\x86Build\Symbian_Tools\Command_Line_Tools\mwccsym2.exe
CC = mwccsym2.exe

c_src_dir = src_c

libstemmer_algorithms = danish dutch english finnish french german hungarian \
			italian \
			norwegian porter portuguese romanian \
			russian spanish swedish turkish


all_algorithms = $(libstemmer_algorithms)

COMPILER_SOURCES = compiler\space.c \
		   compiler\tokeniser.c \
		   compiler\analyser.c \
		   compiler\generator.c \
		   compiler\driver.c \
		   compiler\generator_java.c
COMPILER_HEADERS = compiler\header.h \
		   compiler\syswords.h \
		   compiler\syswords2.h

COMPILER_OBJECTS=$(COMPILER_SOURCES:.c=.o)


CFLAGS=-gccinc -I+include -O2
CPPFLAGS=-strict on

# Just one of the many - we don't (can't) create a list here due to
# having no manual to the make found in the SDK
ONE_SOURCE = c_src/stem_UTF_8_english.c

all: c_src\modules.h group\libstemmer.mmp $(ONE_SOURCE)


clean:
	@del /F $(COMPILER_OBJECTS)
	@del snowball.exe
	@del libstemmer.mak
	@del /F c_src\*.h
	@del /F c_src\*.c


cleangenerated: clean
	@del c_src\*.h
	@del c_src\*.c


snowball.exe: $(COMPILER_OBJECTS)
	$(CC) -o $@ $^


$(COMPILER_OBJECTS): $(COMPILER_HEADERS) $(COMPILER_SOURCES)


compiler\%.o : compiler\%.c
	$(CC) $(CFLAGS) -c $^ -o $@


libstemmer.mak c_src\modules.h group\libstemmer.mmp: mklibstemmermak.pl modules.cfg
	mklibstemmermak.pl


$(ONE_SOURCE): snowball.exe libstemmer.mak
	make -f libstemmer.mak

