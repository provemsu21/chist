BUILD_DIR = build
RELEASE_DIR = $(BUILD_DIR)/release
DEBUG_DIR = $(BUILD_DIR)/debug

all: debug release

release:
	cmake -S . -B $(RELEASE_DIR) -DCMAKE_BUILD_TYPE=Release
	cmake --build $(RELEASE_DIR) -j$(nproc)

debug:
	cmake -S . -B $(DEBUG_DIR) -DCMAKE_BUILD_TYPE=Debug
	cmake --build $(DEBUG_DIR) -j$(nproc)
	ln -sf $(DEBUG_DIR)/compile_commands.json compile_commands.json
