CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17 -I./src -I/usr/include/jsoncpp 
LDFLAGS = -ljsoncpp
OBJ_DIR = build/obj
SRC_DIR = src
TARGET = build/main
HEADERS = $(wildcard $(SRC_DIR)/*.h)
OBJS = $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(filter-out $(SRC_DIR)/main.cpp,$(wildcard $(SRC_DIR)/*.cpp)))
OBJS += $(patsubst $(SRC_DIR)/Core/%.cpp,$(OBJ_DIR)/Core/%.o,$(wildcard $(SRC_DIR)/Core/*.cpp))

.PHONY: all clean test

all: $(TARGET)
test: build/testEmailServer
build/testEmailServer: $(OBJS) $(OBJ_DIR)/testEmailServer.o
	$(CXX) -o $@ $^ $(LDFLAGS) 
$(TARGET): $(OBJS) $(OBJ_DIR)/main.o
	$(CXX)  -o $@ $^ $(LDFLAGS)

$(OBJ_DIR)/testEmailServer.o: tests/testEmailServer.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR) $(TARGET)
