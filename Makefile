#Build Type
#Two possible values
#debug
#release
BUILD_TYPE = release

#Build directory
BUILD_DIR = ./build

#Makefile generated from CMake
GENERATED_MAKEFILE = $(BUILD_DIR)/Makefile

#CMake flags
CMAKE_FLAGS = -DCMAKE_BUILD_TYPE=$(BUILD_TYPE) -DBUILD_LLVM_PASSES=ON

#Check if cmake is installed
CMAKE = $(shell command -v cmake 2> /dev/null)

.PHONY: all
all: $(GENERATED_MAKEFILE)
	@ echo "Running build all..."
	@ $(MAKE) -C $(BUILD_DIR) --no-print-directory all

.PHONY: clean
clean: $(GENERATED_MAKEFILE)
	@ echo "Running clean, removing generated files..."
	@ $(MAKE) -C $(BUILD_DIR) --no-print-directory clean

.PHONY: fullclean
fullclean:
	@ echo "Running fullclean, removing the build results completely..."
	@ rm -rf thirdparty/scipoptsuite/*/
	@ rm -rf $(BUILD_DIR)

.PHONY: $(GENERATED_MAKEFILE)
$(GENERATED_MAKEFILE):
ifndef CMAKE
	$(error Required 'cmake' not found. Please install cmake.)
endif
	@ mkdir -p $(BUILD_DIR)
	@ echo $(CMAKE_FLAGS)
	@ cd $(BUILD_DIR) && $(CMAKE) $(CMAKE_FLAGS) ..
