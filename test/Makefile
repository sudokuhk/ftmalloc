cxx=g++
SERVER = test
#SOURCES = $(wildcard *.cpp)

SOURCES = $(wildcard *.cpp) 

DEP_SOURCES = $(notdir $(SOURCES))
DEP_OBJECTS = $(patsubst %.cpp, %.o, $(DEP_SOURCES))

INC_PATH = $(shell pwd)
LIB_PATH = $(shell pwd)
LIBRARYS= -pthread -lftmalloc
#LIBRARYS= -pthread -ltcmalloc

MACROS = -DC_STRING_FUNC -DLOG_PRINTF -DFT_LOG_DEBUG
SHARED = #-shared -fPIC
CPPFLAGS += -g -O0 -Wno-deprecated -Wnonnull 

INC_PATH += ../src
LIB_PATH += ../src /usr/lib/

INC_DIR = $(patsubst %, -I%, $(INC_PATH))
LIB_DIR = $(patsubst %, -L%, $(LIB_PATH))


all : $(SERVER)

$(SERVER) : $(DEP_OBJECTS)
	$(cxx) $(MACROS) $(CPPFLAGS) $(SHARED) -o $@ $^ $(INC_DIR) $(LIB_DIR) $(LIBRARYS)

.PHONY : clean
clean:
	-rm -f $(SERVER)
	-rm -f *.o
	-rm -f *.d

ifneq "$(MAKECMDGOALS)" "clean"
    include $(DEP_SOURCES:.cpp=.d)  
endif

%.d : 
	@t1=$*; t2=$${t1##*/}; \
	rely_file=""; for file in $(SOURCES); \
	do \
	find=$${file%%$$t2.cpp};	\
	if [ $${#find} != $${#file} ]; then	\
		rely_file=$$file;	\
	fi; 	\
	done;	\
	set -e; rm -f $@; \
	$(cxx) -MM $(CPPFLAGS) $(INC_DIR) $$rely_file > $@.;	\
	cat $@. > $@;	\
	echo "	$(cxx) $(CPPFLAGS) $(MACROS) $(SHARED) -c -o $$t2.o $(INC_DIR) $$rely_file" >> $@; \
	sed 's/\.o/\.d/g' < $@. >> $@; \
	rm -f $@.	;	

