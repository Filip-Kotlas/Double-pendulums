CXX = g++
CXXFLAGS = -Ilibs/imgui -Ilibs/glad/include -Ilibs/glfw/include -Iinclude -std=c++17
LDFLAGS = -Llibs/glfw/lib -lglfw3 -lopengl32 -lgdi32 -luser32 -lshell32

BIN_DIR = build
APP ?= Double-pundulums.exe

IMGUI_SRC = $(wildcard libs/imgui/*.cpp)
GLAD_SRC  = $(wildcard libs/glad/src/*.c)
SRC       = $(wildcard src/*.cpp)

ALL_SRC = $(IMGUI_SRC) $(GLAD_SRC) $(SRC)

OBJ = $(patsubst %.cpp,$(BIN_DIR)/%.o,$(notdir $(filter %.cpp,$(ALL_SRC)))) \
      $(patsubst %.c,$(BIN_DIR)/%.o,$(notdir $(filter %.c,$(ALL_SRC))))

vpath %.cpp $(sort $(dir $(filter %.cpp,$(ALL_SRC))))
vpath %.c   $(sort $(dir $(filter %.c,$(ALL_SRC))))

all: $(BIN_DIR)/$(APP)

$(BIN_DIR)/$(APP): $(OBJ)
	$(CXX) $^ -o $@ $(LDFLAGS)

$(BIN_DIR)/%.o: %.cpp
	@if not exist "$(BIN_DIR)" mkdir "$(BIN_DIR)"
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BIN_DIR)/%.o: %.c
	@if not exist "$(BIN_DIR)" mkdir "$(BIN_DIR)"
	$(CXX) $(CXXFLAGS) -c $< -o $@

.PHONY: run
run: $(BIN_DIR)/$(APP)
	./$(BIN_DIR)/$(APP)

.PHONY: clean
clean:
	@if exist "$(BIN_DIR)" rmdir /S /Q "$(BIN_DIR)"
