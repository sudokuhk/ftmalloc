###########  MakeFile.env  ##########
# Top level pattern, include by Makefile of child directory
# in which variable like TOPDIR, TARGET or LIB may be needed

MAKE=make

dirs:= src test
SUBDIRS := $(dirs)

all:$(TARGET)  subdirs

subdirs:$(SUBDIRS)
	for dir in $(SUBDIRS);\
	do $(MAKE) -C $$dir all||exit 1;\
	done

-include $(DEPENDS)

clean:
	for dir in $(SUBDIRS);\
	do $(MAKE) -C $$dir clean||exit 1;\
	done
