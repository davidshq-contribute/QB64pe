
MINIZ_SRCS := miniz.c

MODP_B64_SRCS := modp_b64.cpp

DATA_PROCESSING_SRCS := \
	compression.cpp \
	encoding.cpp

MINIZ_OBJS := $(call BUILD_OBJ,$(PATH_INTERNAL_C)/parts/data/$(MINIZ_SRCS))

MODP_B64_OBJS := $(call BUILD_OBJ,$(PATH_INTERNAL_C)/parts/data/$(MODP_B64_SRCS))

DATA_PROCESSING_OBJS := $(foreach src,$(DATA_PROCESSING_SRCS),$(call BUILD_OBJ,$(PATH_INTERNAL_C)/parts/data/$(src)))

$(BUILD_OBJ_DIR)/internal/c/parts/data/%.o: $(PATH_INTERNAL_C)/parts/data/%.c | $(BUILD_OBJ_DIR)
	$(call MKDIR_SAFE,$(dir $@))
	$(CC) -O3 $(CFLAGS) -DDEPENDENCY_CONSOLE_ONLY -Wall $< -c -o $@

$(BUILD_OBJ_DIR)/internal/c/parts/data/%.o: $(PATH_INTERNAL_C)/parts/data/%.cpp | $(BUILD_OBJ_DIR)
	$(call MKDIR_SAFE,$(dir $@))
	$(CXX) -O3 $(CXXFLAGS) -DDEPENDENCY_CONSOLE_ONLY -Wall $< -c -o $@

DATA_PROCESSING_LIB := $(call BUILD_LIB,data_processing)

$(DATA_PROCESSING_LIB): $(MINIZ_OBJS) $(MODP_B64_OBJS) $(DATA_PROCESSING_OBJS) | $(BUILD_LIB_DIR)
	$(call MKDIR_SAFE,$(dir $@))
	$(AR) rcs $@ $(MINIZ_OBJS) $(MODP_B64_OBJS) $(DATA_PROCESSING_OBJS)

CLEAN_LIST += $(DATA_PROCESSING_LIB) $(MINIZ_OBJS) $(MODP_B64_OBJS) $(DATA_PROCESSING_OBJS)
