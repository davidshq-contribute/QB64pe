
# GLEW Setup:
# Download the latest release from https://github.com/nigels-com/glew/releases/latest
# Only copy glew.c in src/ to internal/c/parts/core/glew
# Copy the include directory to internal/c/parts/core/glew
# Compile the source using -DGLEW_STATIC
#
# FreeGLUT Setup:
# Although newer version of FreeGLUT (3.x) are available we do not use those (yet).
# This is because the local version has quite a few custom changes that should be moved out first.
# Download the latest 2.x release from https://freeglut.sourceforge.net/
# Copy all .c files from the src directory into internal/c/parts/core/freeglut (after making QB64-PE specific changes)
# Copy the include directory to internal/c/parts/core/freeglut
# Compile the source using -DFREEGLUT_STATIC

FREEGLUT_SRCS := $(wildcard $(PATH_INTERNAL_C)/parts/core/freeglut/*.c)
GLEW_SRCS := $(PATH_INTERNAL_C)/parts/core/glew/glew.c

FREEGLUT_INCLUDE := -I$(PATH_INTERNAL_C)/parts/core/freeglut/include -I$(PATH_INTERNAL_C)/parts/core/glew/include

FREEGLUT_OBJS := $(foreach src,$(FREEGLUT_SRCS),$(call BUILD_OBJ,$(src)))
GLEW_OBJS := $(call BUILD_OBJ,$(GLEW_SRCS))

FREEGLUT_LIB := $(call BUILD_LIB,freeglut)

$(BUILD_OBJ_DIR)/internal/c/parts/core/glew/%.o: $(PATH_INTERNAL_C)/parts/core/glew/%.c | $(BUILD_OBJ_DIR)
	$(call MKDIR_SAFE,$(dir $@))
	$(CC) -O1 $(CFLAGS) $(FREEGLUT_INCLUDE) -DGLEW_STATIC -w $< -c -o $@

$(BUILD_OBJ_DIR)/internal/c/parts/core/freeglut/%.o: $(PATH_INTERNAL_C)/parts/core/freeglut/%.c | $(BUILD_OBJ_DIR)
	$(call MKDIR_SAFE,$(dir $@))
	$(CC) -O3 $(CFLAGS) $(FREEGLUT_INCLUDE) -DFREEGLUT_STATIC -DHAVE_UNISTD_H -DHAVE_FCNTL_H -w $< -c -o $@

$(FREEGLUT_LIB): $(FREEGLUT_OBJS) | $(BUILD_LIB_DIR)
	$(call MKDIR_SAFE,$(dir $@))
	$(AR) rcs $@ $(FREEGLUT_OBJS)

QB_CORE_LIB := $(FREEGLUT_LIB)

CXXFLAGS += $(FREEGLUT_INCLUDE)

CLEAN_LIST += $(FREEGLUT_LIB) $(FREEGLUT_OBJS) $(GLEW_OBJS)
