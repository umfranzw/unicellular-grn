PROG = grn
INCLUDE_DIR = include
SRC_DIR = src
HDR_FILES = gene.h protein.h
SRC_FILES = main.cpp gene.cpp protein.cpp

# Tack on the appropriate dir name (cut down on the amount of typing required above)
HDRS = $(patsubst %.h, $(INCLUDE_DIR)/%.h, $(HDR_FILES))
SRCS = $(patsubst %.cpp, $(SRC_DIR)/%.cpp, $(SRC_FILES))

# directory to store object files
OBJDIR = object
# names of object files
OBJS = $(patsubst %.cpp, $(OBJDIR)/%.o, $(SRCS)) 

# name of the compiler
CC = g++
# additional compiler flags to pass in
CFLAGS = -fopenmp -Wall --std=c++11 -L. -I$(INCLUDE_DIR)
# libraries for the linker
LIBS = 

####################
# Compiling rules: #
####################
# WARNING: *must* have a tab before each definition

# invoked when "make" is run
all : $(OBJDIR) $(PROG)

# links object files into executable
$(PROG) : $(OBJS)
	$(CC) $(CFLAGS) $(patsubst %.o, $(OBJDIR)/%.o, $(notdir $^)) -o $(PROG) $(LIBS)

# compiles source files into object files
object/%.o : %.cpp $(HDRS)
	$(CC) -c $(CFLAGS) $< -o $(OBJDIR)/$(notdir $@) $(LIBS)

# creates directory to store object files
$(OBJDIR) :
	mkdir -p $@/

# cleans up object files and executable
clean:
	rm -rf object/
	rm -f $(PROG)
