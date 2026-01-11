
ifeq ($(OS),lnx)
	GAMEPAD_SRCS := Gamepad_linux.c Gamepad_private.c
endif

ifeq ($(OS),win)
	GAMEPAD_SRCS := Gamepad_windows_mm.c Gamepad_private.c
endif

ifeq ($(OS),osx)
	GAMEPAD_SRCS := Gamepad_macosx.c Gamepad_private.c
endif

GAMECONTROLLER_SRCS := game_controller.cpp

GAMEPAD_OBJS := $(foreach src,$(GAMEPAD_SRCS),$(call BUILD_OBJ,$(PATH_INTERNAL_C)/parts/input/game_controller/libstem_gamepad/$(src)))

GAMECONTROLLER_OBJS := $(call BUILD_OBJ,$(PATH_INTERNAL_C)/parts/input/game_controller/$(GAMECONTROLLER_SRCS))

$(BUILD_OBJ_DIR)/internal/c/parts/input/game_controller/libstem_gamepad/%.o: $(PATH_INTERNAL_C)/parts/input/game_controller/libstem_gamepad/%.c | $(BUILD_OBJ_DIR)
	$(call MKDIR_SAFE,$(dir $@))
	$(CC) -O2 $(CFLAGS) -Wall $< -c -o $@

$(BUILD_OBJ_DIR)/internal/c/parts/input/game_controller/%.o: $(PATH_INTERNAL_C)/parts/input/game_controller/%.cpp | $(BUILD_OBJ_DIR)
	$(call MKDIR_SAFE,$(dir $@))
	$(CXX) -O2 $(CXXFLAGS) -Wall $< -c -o $@

QB_DEVICE_INPUT_LIB := $(call BUILD_LIB,game_controller)

$(QB_DEVICE_INPUT_LIB): $(GAMEPAD_OBJS) $(GAMECONTROLLER_OBJS) | $(BUILD_LIB_DIR)
	$(call MKDIR_SAFE,$(dir $@))
	$(AR) rcs $@ $(GAMEPAD_OBJS) $(GAMECONTROLLER_OBJS)

CLEAN_LIST += $(QB_DEVICE_INPUT_LIB) $(GAMEPAD_OBJS) $(GAMECONTROLLER_OBJS)

