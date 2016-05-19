cxx=g++
TARGET = libftmalloc.so
SOURCES = $(wildcard *.cpp)
OBJECTS = $(patsubst %.cpp,%.o, $(SOURCES))
INC_DIR = $(shell pwd)
LIBRARYS= -pthread
MACROS = -DC_STRING_FUNC -DLOG_PRINTF -DFT_LOG_ERROR
SHARED = -shared -fPIC

all : $(TARGET)

$(TARGET) : $(OBJECTS)
	$(cxx) $(MACROS) -g $(SHARED) -o ../$@ $(OBJECTS) -I$(INC_DIR) $(LIBRARYS)

.PHONY : clean
clean:
	-rm -f $(TARGET)
	-rm -f *.o

%.o : %.cpp
	$(cxx) $(MACROS) $(SHARED) -c -o $@ -I. $<

