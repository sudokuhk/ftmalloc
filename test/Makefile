cxx=g++
TARGET = main
SOURCES = $(wildcard *.cpp)
OBJECTS = $(patsubst %.cpp,%.o, $(SOURCES))
INC_DIR = -I$(shell pwd)
LIBRARYS= -pthread -lftmalloc
MACROS = -DC_STRING_FUNC
LIB_PATH = -L../
SHARED = #-shared -fPIC

INC_DIR += -I../src/

all : $(TARGET)

$(TARGET) : $(OBJECTS)
	$(cxx) $(MACROS) -g $(SHARED) -o ../$@ $(OBJECTS) $(INC_DIR) $(LIB_PATH) $(LIBRARYS)

.PHONY : clean
clean:
	-rm -f $(TARGET)
	-rm -f *.o

%.o : %.cpp
	$(cxx) $(MACROS) $(SHARED) -c -o $@ $(INC_DIR) $<

