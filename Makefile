SOURCE_DIR = src/
CPP = /opt/intel/bin/icpc

BINARY = roundendblock_mm_i386.so
BUILD_OBJ_DIR=Release/obj

OBJECTS = $(SOURCE_DIR)main.cpp $(SOURCE_DIR)memory.cpp $(SOURCE_DIR)meta_api.cpp $(SOURCE_DIR)h_export.cpp

CFLAGS = -mia32 -O3 -static-intel -fasm-blocks -no-intel-extensions\
	-falign-functions=2 -funroll-loops -fno-rtti -fno-exceptions\
	-fno-stack-protector -fno-builtin -shared -static-libgcc\
	-Wno-unknown-pragmas -s -g0 -fvisibility=hidden -fvisibility-inlines-hidden\
	-DNDEBUG

LDFLAGS = -lm -ldl -lstdc++ -std=c++0x
INCLUDE = -I$(SOURCE_DIR) -I. -I$(SOURCE_DIR)/sdk

OBJ_LINUX := $(OBJECTS:$(SOURCE_DIR)/%.cpp=$(BUILD_OBJ_DIR)/%.o)

$(BUILD_OBJ_DIR)/%.o : $(SOURCE_DIR)/%.cpp
	$(CPP) $(INCLUDE) $(CFLAGS) -o $@ -c $<

default: all

dirs:
	mkdir -p Release
	mkdir -p $(BUILD_OBJ_DIR)/src

binary: $(OBJ_LINUX)
	rm -f Release/*.*
	$(CPP) $(INCLUDE) $(CFLAGS) $(OBJ_LINUX) $(LDFLAGS) -o Release/$(BINARY)

all: dirs binary

.PHONY: debug clean

debug:
	$(MAKE) all DEBUG=true

clean:
	rm -rf ./Release
