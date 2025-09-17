CXX = g++
PROFFLAGS ?=
CXXFLAGS = -Iinclude -std=c++17 $(PROFFLAGS)
LDFLAGS = $(PROFFLAGS)
OUTPUT ?= output
BIN_DIR = build
APP ?= Double-pendulums

SRC = $(wildcard src/*.cpp)

OBJ = $(patsubst %.cpp,$(BIN_DIR)/%.o,$(notdir $(filter %.cpp,$(SRC)))) \
      $(patsubst %.c,$(BIN_DIR)/%.o,$(notdir $(filter %.c,$(SRC))))

vpath %.cpp $(sort $(dir $(filter %.cpp,$(SRC))))
vpath %.c   $(sort $(dir $(filter %.c,$(SRC))))

all: $(BIN_DIR)/$(APP)

$(BIN_DIR)/$(APP): $(OBJ)
	@mkdir -p $(BIN_DIR)
	$(CXX) $^ -o $@ $(LDFLAGS)

$(BIN_DIR)/%.o: %.cpp
	@mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BIN_DIR)/%.o: %.c
	@mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

.PHONY: run
run: $(BIN_DIR)/$(APP)
	./$(BIN_DIR)/$(APP) $(OUTPUT)

.PHONY: clean
clean:
	rm -rf $(BIN_DIR)
