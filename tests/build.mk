
TESTS :=

TEST_CFLAGS-y := -I'./tests/c/include' \
			   -I$(PATH_LIBQB)/include \
			   -g -std=gnu++11

TEST_CFLAGS-$(win) += -mconsole -static-libgcc -static-libstdc++

TEST_DEF_OBJS := tests/c/test.o

# Defines the list of test sets
TESTS += buffer
TESTS += http
TESTS += gfs
TESTS += logging
TESTS += threading
TESTS += graphics
TESTS += audio

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
				$(PATH_LIBQB)/src/filepath.cpp

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
				$(PATH_LIBQB)/src/qbs.cpp

# Audio tests may be skipped if audio library is not available
# Uncomment below to link against audio library (requires audio to be built first):
# audio.src-y += $(PATH_INTERNAL_C)/parts/audio/audio.cpp
# audio.libs-y += $(AUDIO_LIB)


TEST_OBJS := $(TEST_DEF_OBJS)
TEST_OBJS += $(foreach test,$(TESTS),$(filter ./tests/c/%,$($(test)).objs-y))

TEST_TESTS :=

define TEST_template
TEST_TESTS += ./tests/exes/cpp/$(1)_test$(EXTENSION)
tests/exes/cpp/$(1)_test$(EXTENSION): $$(TEST_DEF_OBJS) $$($(1).src-y) $$($(1).exe-libs-y) | tests/exes/cpp
	$$(CXX) $$(TEST_CFLAGS-y) $$($(1).cflags-y) $$^ -o $$@ $$($(1).exe-libs-y) $$($(1).libs-y)
endef

$(foreach test,$(TESTS),$(eval $(call TEST_template,$(test))))

CLEAN_LIST += $(TEST_OBJS)

tests/exes:
	$(MKDIR) $(call FIXPATH,$@)

tests/exes/cpp: | tests/exes
	$(MKDIR) $(call FIXPATH,$@)

PHONY += build-tests
build-tests: $(TEST_DEF_OBJS) $(TEST_TESTS)
