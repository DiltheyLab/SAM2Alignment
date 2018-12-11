## To build:
##    'make all'
## To clean:
##    'make clean'


INCS = 
LIBS = 

MKDIR_P = mkdir -p

.PHONY: directories
	
# END LIBRARY SETTINGS

#
# Directions for users
#
default:
	@echo "Directions: "
	@echo
	@echo " To build:"
	@echo "    make all"
	@echo
	@echo " To clean:"
	@echo "    make clean"
	@echo

#
# object and binary dirs  
#

DIR_OBJ = .
DIR_BIN = .

CXX    = g++
COPTS  = -ggdb -O2 -std=gnu++0x -fstack-protector-all
CFLAGS = 
COMPILE = $(CXX) $(INCS) $(CFLAGS) $(COPTS)
VPATH = 
        
OBJS = 
        
#
# list executable file names
#
EXECS = sam2alignment

OUT_DIR = .

directories: ${OUT_DIR}


all: directories $(EXECS)

$(EXECS): $(OBJS)
	$(foreach EX, $(EXECS), $(COMPILE) $(EX).cpp -c -o $(DIR_OBJ)/$(EX).o;)
	$(foreach EX, $(EXECS), $(COMPILE) $(OBJS) $(DIR_OBJ)/$(EX).o -o $(DIR_BIN)/$(EX) $(LIBS);)

$(DIR_OBJ)/%.o: %.cpp %.h
	$(COMPILE) $< -c -o $@


#
# odds and ends
#
clean:
	/bin/rm sam2alignment

${OUT_DIR}:
	${MKDIR_P} ${OUT_DIR}

