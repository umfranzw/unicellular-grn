EXE = grn

SRC_DIR = src
INC_DIR = include
OBJ_DIR = obj

SRCS = $(wildcard $(SRC_DIR)/*.cpp)
#HDRS = $(wildcard $(INC_DIR)/*.h)
OBJ = $(SRCS:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)

CC = g++
CPPFLAGS += -Wall -O3 -fopenmp --std=c++11 -I$(INC_DIR)
LDFLAGS += -O3 -fopenmp
LDLIBS += -lm -lsqlite3

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

