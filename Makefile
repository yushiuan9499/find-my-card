CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17 -I./src
OBJ_DIR = build/obj
SRC_DIR = src
TARGET = build/main
HEADERS = $(wildcard $(SRC_DIR)/*.h)
OBJS = $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(wildcard $(SRC_DIR)/*.cpp))

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS) 
	$(CXX) $(CXXFLAGS) -o $@ $^

clean:
	rm -rf $(OBJ_DIR) $(TARGET)
