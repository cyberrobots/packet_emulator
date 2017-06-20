# Dummy builder for scramblenet project.

#FORWARD_TRAFFIC = -DFORWARD_UNMATCHED

CC 		= gcc
LD          	= gcc
CFLAGS  	= -g -O0 -Wall -D_REENTRANT -DTIME_LOG 
LDFLAGS 	= -lpthread -lrt -lm
OUTPUTFILE      = p_emu.bin

#Source Directories
# Header file paths
INCLUDES	= ./inc/ \
		  ./libs/llib/ \

# Source file paths
SOURCE_PATHS	= ./src \
		  ./libs/llib \
                  
# Gather all header files                  
INCLUDE_FILES	=$(foreach d, $(INCLUDES), $(wildcard $d*.h))
# Gather all header paths and add -I flag
INC_PARAMS	=$(foreach d, $(INCLUDES), -I $d)
# Gather all source files
SOURCE_FILES   	= $(foreach d, $(SOURCE_PATHS), $(wildcard $d/*.c))
# Gather all object files (source files changing ending)
OBJECTS         = $(patsubst %.c,%.o,$(SOURCE_FILES))
# Rule for converting source files to object files (compile)
%.o: %.c $(INCLUDE_FILES)
	$(CC) -c $(CFLAGS) $(INC_PARAMS) -o $@ $<

# Make all rule (linking)
all : $(OBJECTS)
	@echo "Linking..."
	$(LD) $(OBJECTS) $(LDFLAGS) -o $(OUTPUTFILE) -Wl,-Map=$(OUTPUTFILE).map
	size $(OUTPUTFILE)

.PHONY: clean

# Make clean rule (removing binaries and object files)
clean:
	@echo "Clean...."
	rm -rf $(OBJECTS)
	rm -rf ./$(OUTPUTFILE)
	rm -rf ./$(OUTPUTFILE).map

# Make info rule (just show some stuff)
info: 
	@echo "Header Files....."
	@echo $(INCLUDE_FILES)
	@echo "Source Files....."
	@echo $(SOURCE_FILES)
