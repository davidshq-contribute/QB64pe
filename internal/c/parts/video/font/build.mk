
# We use the flat-directory compilation for FreeType as explained in:
# https://github.com/freetype/freetype/blob/master/docs/INSTALL.ANY
#
# When updating the library:
# 1. Flatten all directories inside "src" except "tools". Omit contents of "tools" entirely.
# 2. Then only copy all .c & .h files except:
#       autofit.c, bdf.c, cff.c, ftbase.c, ftcache.c, gxvalid.c, gxvfgen.c,
#       otvalid.c, pcf.c, pfr.c, pshinter.c, psnames.c, psaux.c, raster.c, sdf.c,
#       sfnt.c, smooth.c, svg.c, truetype.c, type1.c, type1cid.c, type42.c
# 3. Copy the FreeType "include" directory *without* flattening!
# 4. Include <freetype/internal/compiler-macros.h> in "ftzopen.h".
# 5. Include <freetype/config/ftstdlib.h> in "zutil.h".

FREETYPE_SRCS := $(wildcard $(PATH_INTERNAL_C)/parts/video/font/freetype/*.c)

FREETYPE_INCLUDE := -I$(PATH_INTERNAL_C)/parts/video/font/freetype/include

FREETYPE_OBJS := $(foreach src,$(FREETYPE_SRCS),$(call BUILD_OBJ,$(src)))

FREETYPE_LIB := $(call BUILD_LIB,freetype)

FONT_SRCS := \
	font.cpp \
	hashing.cpp

FONT_OBJS := $(foreach src,$(FONT_SRCS),$(call BUILD_OBJ,$(PATH_INTERNAL_C)/parts/video/font/$(src)))

FONT_LIB := $(call BUILD_LIB,font)

FONT_STUB_SRCS := stub_font.cpp

FONT_STUB_OBJS := $(call BUILD_OBJ,$(PATH_INTERNAL_C)/parts/video/font/$(FONT_STUB_SRCS))

$(BUILD_OBJ_DIR)/internal/c/parts/video/font/freetype/%.o: $(PATH_INTERNAL_C)/parts/video/font/freetype/%.c | $(BUILD_OBJ_DIR)
	$(call MKDIR_SAFE,$(dir $@))
	$(CC) -O3 $(CFLAGS) $(FREETYPE_INCLUDE) -DFT2_BUILD_LIBRARY -w $< -c -o $@

$(BUILD_OBJ_DIR)/internal/c/parts/video/font/%.o: $(PATH_INTERNAL_C)/parts/video/font/%.cpp | $(BUILD_OBJ_DIR)
	$(call MKDIR_SAFE,$(dir $@))
	$(CXX) -O3 $(CXXFLAGS) $(FREETYPE_INCLUDE) -DDEPENDENCY_CONSOLE_ONLY -w $< -c -o $@

$(FREETYPE_LIB): $(FREETYPE_OBJS) | $(BUILD_LIB_DIR)
	$(call MKDIR_SAFE,$(dir $@))
	$(AR) rcs $@ $(FREETYPE_OBJS)

$(FONT_LIB): $(FONT_OBJS) | $(BUILD_LIB_DIR)
	$(call MKDIR_SAFE,$(dir $@))
	$(AR) rcs $@ $(FONT_OBJS)

FREETYPE_EXE_LIBS := $(FONT_LIB) $(FREETYPE_LIB)

CLEAN_LIST += $(FREETYPE_LIB) $(FONT_LIB) $(FREETYPE_OBJS) $(FONT_OBJS) $(FONT_STUB_OBJS)
