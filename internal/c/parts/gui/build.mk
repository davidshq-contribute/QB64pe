
TFD_SRCS := \
	tinyfiledialogs.c

GUI_SRCS := \
	gui.cpp

TFD_OBJS := $(call BUILD_OBJ,$(PATH_INTERNAL_C)/parts/gui/$(TFD_SRCS))

GUI_OBJS := $(call BUILD_OBJ,$(PATH_INTERNAL_C)/parts/gui/$(GUI_SRCS))

$(BUILD_OBJ_DIR)/internal/c/parts/gui/%.o: $(PATH_INTERNAL_C)/parts/gui/%.c | $(BUILD_OBJ_DIR)
	$(call MKDIR_SAFE,$(dir $@))
	$(CC) -O2 $(CFLAGS) -DDEPENDENCY_CONSOLE_ONLY -Wall $< -c -o $@

$(BUILD_OBJ_DIR)/internal/c/parts/gui/%.o: $(PATH_INTERNAL_C)/parts/gui/%.cpp | $(BUILD_OBJ_DIR)
	$(call MKDIR_SAFE,$(dir $@))
	$(CXX) -O2 $(CXXFLAGS) -DDEPENDENCY_CONSOLE_ONLY -Wall $< -c -o $@

EXE_LIBS += $(TFD_OBJS) $(GUI_OBJS)

CLEAN_LIST += $(TFD_OBJS) $(GUI_OBJS)
