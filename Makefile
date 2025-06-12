CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17 -I./src
OBJ_DIR = build/obj
SRC_DIR = src
TARGET = build/main
HEADERS = $(wildcard $(SRC_DIR)/*.h)
OBJS = $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(filter-out $(SRC_DIR)/main.cpp,$(wildcard $(SRC_DIR)/*.cpp)))

.PHONY: all clean test

all: $(TARGET)
test: build/testEmailServer
build/testEmailServer: $(OBJS) tests/testEmailServer.cpp
	$(CXX) $(CXXFLAGS) -o $@ $^

$(TARGET): $(OBJS) $(SRC_DIR)/main.cpp
	$(CXX) $(CXXFLAGS) -o $@ $^

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR) $(TARGET)
