# Compilers
CXX_GCC = g++
CXX_CLANG = clang++
WINDRES = windres

# Source files and resource file
SRCS = src/crun.cpp src/options.cpp src/compiler.cpp src/utils.cpp src/version.cpp
RES_SRC = res/crun.rc
RES = res/crun.res

# Output directory and target executables
BIN_DIR = bin
TARGET_GCC = $(BIN_DIR)/crun_gcc.exe
TARGET_CLANG = $(BIN_DIR)/crun_clang.exe
TARGET_TEST_GCC = $(BIN_DIR)/crun_test.exe
TARGET_TEST_CLANG = $(BIN_DIR)/crun_test_clang.exe
TARGET_FINAL = $(BIN_DIR)/crun.exe

# Common flags
LDFLAGS = -lkernel32 -luser32 -lshell32 -lmsvcrt -lshlwapi -static
COMMON_FLAGS = -s -fno-exceptions -fno-rtti -ffunction-sections -fdata-sections -Wl,--gc-sections,--strip-all

# Compiler-specific optimization flags
GCC_OPT_FLAGS = -Os
CLANG_OPT_FLAGS = -Oz

# Phony targets
.PHONY: all gcc clang test clean link

# Default target
all: gcc clang link

# Resource compilation
$(RES): $(RES_SRC)
	@echo "Compiling resources..."
	$(WINDRES) -i $(RES_SRC) -o $(RES) --output-format=coff

# Release builds
gcc: $(TARGET_GCC)

clang: $(TARGET_CLANG)

$(TARGET_GCC): $(SRCS) $(RES)
	@echo "Building release version with GCC..."
	$(CXX_GCC) $(SRCS) $(RES) -o $@ $(GCC_OPT_FLAGS) $(COMMON_FLAGS) $(LDFLAGS)

$(TARGET_CLANG): $(SRCS) $(RES)
	@echo "Building release version with Clang..."
	$(CXX_CLANG) $(SRCS) $(RES) -o $@ $(CLANG_OPT_FLAGS) $(COMMON_FLAGS) $(LDFLAGS)

# Test builds
test: test-gcc test-clang

test-gcc: $(TARGET_TEST_GCC)

test-clang: $(TARGET_TEST_CLANG)

$(TARGET_TEST_GCC): $(SRCS) $(RES)
	@echo "Building test version with GCC..."
	$(CXX_GCC) $(SRCS) $(RES) -o $@ $(GCC_OPT_FLAGS) $(COMMON_FLAGS) $(LDFLAGS)

$(TARGET_TEST_CLANG): $(SRCS) $(RES)
	@echo "Building test version with Clang..."
	$(CXX_CLANG) $(SRCS) $(RES) -o $@ $(CLANG_OPT_FLAGS) $(COMMON_FLAGS) $(LDFLAGS)

# Create final executable
link: $(TARGET_CLANG)
	@echo "Creating final crun.exe from Clang build..."
	@cp -f $(TARGET_CLANG) $(TARGET_FINAL)

# Clean up build files
clean:
	@echo "Cleaning up build files..."
	@rm -f $(subst \,/,$(TARGET_GCC)) $(subst \,/,$(TARGET_CLANG)) $(subst \,/,$(TARGET_TEST_GCC)) $(subst \,/,$(TARGET_TEST_CLANG)) $(subst \,/,$(TARGET_FINAL)) $(subst \,/,$(RES))

