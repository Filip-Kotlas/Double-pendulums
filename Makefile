CXX = nvcc
PROFFLAGS =
CXXFLAGS = -Iinclude -std=c++17 $(PROFFLAGS)

BIN_DIR = build
APP ?= Double-pendulums

SRC = $(wildcard src/*.cu) $(wildcard src/*.cpp) $(wildcard src/*.c)

OBJ = $(patsubst %.cu,$(BIN_DIR)/%.o,$(notdir $(filter %.cu,$(SRC)))) \
      $(patsubst %.c,$(BIN_DIR)/%.o,$(notdir $(filter %.c,$(SRC)))) \
      $(patsubst %.cpp,$(BIN_DIR)/%.o,$(notdir $(filter %.cpp,$(SRC))))

vpath %.cu $(sort $(dir $(filter %.cu,$(SRC))))
vpath %.c   $(sort $(dir $(filter %.c,$(SRC))))
vpath %.cpp $(sort $(dir $(filter %.cpp,$(SRC))))


all: $(BIN_DIR)/$(APP)

$(BIN_DIR)/$(APP): $(OBJ)
	@mkdir -p $(BIN_DIR)
	$(CXX) $^ -o $@ $(LDFLAGS)

$(BIN_DIR)/%.o: %.cu
	@mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BIN_DIR)/%.o: %.c
	@mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BIN_DIR)/%.o: %.cpp
	@mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

.PHONY: run
run: $(BIN_DIR)/$(APP)
	./$(BIN_DIR)/$(APP)

.PHONY: clean
clean:
	rm -rf $(BIN_DIR)
