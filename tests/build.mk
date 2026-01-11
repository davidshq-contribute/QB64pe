
TESTS :=

TEST_CFLAGS-y := -I'./tests/c/include' \
			   -I$(PATH_LIBQB)/include \
			   -g -std=gnu++11

TEST_CFLAGS-$(win) += -mconsole -static-libgcc -static-libstdc++

TEST_DEF_OBJS := $(call BUILD_OBJ,tests/c/test.cpp)

# Defines the list of test sets
TESTS += buffer
TESTS += http
TESTS += gfs
TESTS += logging
TESTS += threading
TESTS += graphics
TESTS += audio
TESTS += qbs_comprehensive

# Describe how to build each test
buffer.src-y := ./tests/c/buffer.cpp \
				$(PATH_LIBQB)/src/buffer.cpp

http.src-y := ./tests/c/http.cpp \
				$(PATH_LIBQB)/src/http.cpp \
				$(PATH_LIBQB)/src/buffer.cpp \
				$(PATH_LIBQB)/src/threading-$(PLATFORM).cpp \
				$(PATH_LIBQB)/src/threading.cpp

http.cflags-y := $(CURL_CXXFLAGS)
http.libs-y := $(CURL_CXXLIBS)
http.exe-libs-y := $(CURL_EXE_LIBS)

http.libs-$(lnx) += -lpthread
http.libs-$(win) += -lws2_32

# GFS tests
gfs.src-y := ./tests/c/gfs.cpp \
				$(PATH_LIBQB)/src/gfs.cpp \
				$(PATH_LIBQB)/src/qbs.cpp \
				$(PATH_LIBQB)/src/filepath.cpp \
				$(PATH_LIBQB)/src/error_handle.cpp \
				$(PATH_LIBQB)/src/buffer.cpp

# Logging tests
logging.src-y := ./tests/c/logging.cpp \
				$(PATH_LIBQB)/src/logging/logging.cpp \
				$(PATH_LIBQB)/src/logging/handlers/fp_handler.cpp \
				$(PATH_LIBQB)/src/logging/stacktrace.cpp \
				$(PATH_LIBQB)/src/qbs.cpp \
				$(PATH_LIBQB)/src/datetime.cpp \
				$(PATH_LIBQB)/src/threading-$(PLATFORM).cpp \
				$(PATH_LIBQB)/src/threading.cpp

logging.libs-$(lnx) += -lpthread
logging.libs-$(win) += -lws2_32

# Threading tests
threading.src-y := ./tests/c/threading.cpp \
				$(PATH_LIBQB)/src/threading-$(PLATFORM).cpp \
				$(PATH_LIBQB)/src/threading.cpp

threading.libs-$(lnx) += -lpthread
threading.libs-$(win) += -lws2_32

# Graphics tests
graphics.src-y := ./tests/c/graphics.cpp \
				$(PATH_LIBQB)/src/graphics.cpp \
				$(PATH_LIBQB)/src/qbs.cpp \
				$(PATH_LIBQB)/src/error_handle.cpp \
				$(PATH_LIBQB)/src/qblist.cpp \
				$(PATH_LIBQB)/src/rounding.cpp

# Audio tests
# Note: Audio tests are limited to functions that don't require full audio initialization
# Full audio functionality requires the audio library which has many dependencies
audio.src-y := ./tests/c/audio.cpp \
				$(PATH_LIBQB)/src/qbs.cpp \
				$(PATH_LIBQB)/src/error_handle.cpp \
				$(PATH_LIBQB)/src/buffer.cpp

# Audio tests may be skipped if audio library is not available
# Uncomment below to link against audio library (requires audio to be built first):
# audio.src-y += $(PATH_INTERNAL_C)/parts/audio/audio.cpp
# audio.libs-y += $(AUDIO_LIB)

# QBS Comprehensive tests
qbs_comprehensive.src-y := ./tests/c/qbs_comprehensive.cpp \
				$(PATH_LIBQB)/src/qbs.cpp \
				$(PATH_LIBQB)/src/qbs_str.cpp \
				$(PATH_LIBQB)/src/string_functions.cpp \
				$(PATH_LIBQB)/src/error_handle.cpp \
				$(PATH_LIBQB)/src/buffer.cpp

TEST_OBJS := $(TEST_DEF_OBJS)
TEST_OBJS += $(foreach test,$(TESTS),$(filter ./tests/c/%,$($(test)).objs-y))

TEST_TESTS :=

# Pattern rule for test source files
$(BUILD_OBJ_DIR)/tests/c/%.o: tests/c/%.cpp | $(BUILD_OBJ_DIR)
	$(call MKDIR_SAFE,$(dir $@))
	$(CXX) $(TEST_CFLAGS-y) $< -c -o $@

# Helper to convert source files to object files
# For test sources, compile to build directory
# For libqb sources, they should already be objects from libqb/build.mk
test_src_to_obj = $(foreach src,$1,$(if $(filter ./tests/c/%,$(src)),$(call BUILD_OBJ,$(src)),$(call BUILD_OBJ,$(src))))

define TEST_template
TEST_TESTS += $(BUILD_DIR)/tests/exes/cpp/$(1)_test$(EXTENSION)
$(BUILD_DIR)/tests/exes/cpp/$(1)_test$(EXTENSION): $$(TEST_DEF_OBJS) $$(call test_src_to_obj,$$($(1).src-y)) $$($(1).exe-libs-y) | $(BUILD_DIR)/tests/exes/cpp
	$$(CXX) $$(TEST_CFLAGS-y) $$($(1).cflags-y) $$^ -o $$@ $$($(1).exe-libs-y) $$($(1).libs-y)
endef

$(foreach test,$(TESTS),$(eval $(call TEST_template,$(test))))

CLEAN_LIST += $(TEST_OBJS) $(TEST_TESTS)

$(BUILD_DIR)/tests/exes:
	$(MKDIR) $(call FIXPATH,$@)

$(BUILD_DIR)/tests/exes/cpp: | $(BUILD_DIR)/tests/exes
	$(MKDIR) $(call FIXPATH,$@)

PHONY += build-tests
build-tests: $(TEST_DEF_OBJS) $(TEST_TESTS)
