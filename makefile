EXE = grn

SRC_DIR = src
INC_DIR = include
OBJ_DIR = obj
HAVE_ZMQ = 1

SRCS = $(wildcard $(SRC_DIR)/*.cpp)
#HDRS = $(wildcard $(INC_DIR)/*.h)
OBJ = $(SRCS:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)

CC = clang++
CPPFLAGS += -g -Wall --std=c++11 -I$(INC_DIR) -DHAVE_ZMQ=$(HAVE_ZMQ)
LDFLAGS += 
LDLIBS += -lm -lsqlite3

ifeq ($(HAVE_ZMQ),1)
LDLIBS += -lzmq
endif

.PHONY: all clean

all: $(OBJ_DIR) $(EXE)

$(EXE): $(OBJ)
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CC) $(CPPFLAGS) -c $< -o $@

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

clean:
	rm -r $(OBJ_DIR)
	rm $(EXE)

