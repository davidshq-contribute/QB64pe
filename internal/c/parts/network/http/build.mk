
ifeq ($(OS),win)
# This version is only used for Windows. Linux and macOS use the library provided by their system.
#
# When updating the library remove everything except the "include" directory and the "lib" directory.

CURL_SRCS := $(wildcard $(PATH_INTERNAL_C)/parts/network/http/curl/lib/*.c \
	$(PATH_INTERNAL_C)/parts/network/http/curl/lib/curlx/*.c \
	$(PATH_INTERNAL_C)/parts/network/http/curl/lib/vauth/*.c \
	$(PATH_INTERNAL_C)/parts/network/http/curl/lib/vquic/*.c \
	$(PATH_INTERNAL_C)/parts/network/http/curl/lib/vssh/*.c \
	$(PATH_INTERNAL_C)/parts/network/http/curl/lib/vtls/*.c)

CURL_INCLUDES := -I$(PATH_INTERNAL_C)/parts/network/http/curl/include

CURL_OBJS := $(foreach src,$(CURL_SRCS),$(call BUILD_OBJ,$(src)))

CURL_LIB := $(call BUILD_LIB,libcurl)

CURL_CFLAGS := -DBUILDING_LIBCURL -DCURL_STATICLIB -DHTTP_ONLY -DUSE_SCHANNEL -DUSE_WINDOWS_SSPI

$(BUILD_OBJ_DIR)/internal/c/parts/network/http/curl/lib/%.o: $(PATH_INTERNAL_C)/parts/network/http/curl/lib/%.c | $(BUILD_OBJ_DIR)
	$(call MKDIR_SAFE,$(dir $@))
	$(CC) -O3 $(CFLAGS) $(CURL_INCLUDES) $(CURL_CFLAGS) -w $< -c -o $@

$(BUILD_OBJ_DIR)/internal/c/parts/network/http/curl/lib/curlx/%.o: $(PATH_INTERNAL_C)/parts/network/http/curl/lib/curlx/%.c | $(BUILD_OBJ_DIR)
	$(call MKDIR_SAFE,$(dir $@))
	$(CC) -O3 $(CFLAGS) $(CURL_INCLUDES) $(CURL_CFLAGS) -w $< -c -o $@

$(BUILD_OBJ_DIR)/internal/c/parts/network/http/curl/lib/vauth/%.o: $(PATH_INTERNAL_C)/parts/network/http/curl/lib/vauth/%.c | $(BUILD_OBJ_DIR)
	$(call MKDIR_SAFE,$(dir $@))
	$(CC) -O3 $(CFLAGS) $(CURL_INCLUDES) $(CURL_CFLAGS) -w $< -c -o $@

$(BUILD_OBJ_DIR)/internal/c/parts/network/http/curl/lib/vquic/%.o: $(PATH_INTERNAL_C)/parts/network/http/curl/lib/vquic/%.c | $(BUILD_OBJ_DIR)
	$(call MKDIR_SAFE,$(dir $@))
	$(CC) -O3 $(CFLAGS) $(CURL_INCLUDES) $(CURL_CFLAGS) -w $< -c -o $@

$(BUILD_OBJ_DIR)/internal/c/parts/network/http/curl/lib/vssh/%.o: $(PATH_INTERNAL_C)/parts/network/http/curl/lib/vssh/%.c | $(BUILD_OBJ_DIR)
	$(call MKDIR_SAFE,$(dir $@))
	$(CC) -O3 $(CFLAGS) $(CURL_INCLUDES) $(CURL_CFLAGS) -w $< -c -o $@

$(BUILD_OBJ_DIR)/internal/c/parts/network/http/curl/lib/vtls/%.o: $(PATH_INTERNAL_C)/parts/network/http/curl/lib/vtls/%.c | $(BUILD_OBJ_DIR)
	$(call MKDIR_SAFE,$(dir $@))
	$(CC) -O3 $(CFLAGS) $(CURL_INCLUDES) $(CURL_CFLAGS) -w $< -c -o $@

$(CURL_LIB): $(CURL_OBJS) | $(BUILD_LIB_DIR)
	$(call MKDIR_SAFE,$(dir $@))
	$(AR) rcs $@ $(CURL_OBJS)

CLEAN_LIST += $(CURL_LIB) $(CURL_OBJS)

CURL_EXE_LIBS := $(CURL_LIB)

CURL_CXXFLAGS += $(CURL_CFLAGS)
CURL_CXXFLAGS += $(CURL_INCLUDES)

CURL_CXXLIBS += -lcrypt32 -lbcrypt -lwldap32 -lws2_32 -lsecur32

else

CURL_EXE_LIBS :=
CURL_CXXLIBS :=
CURL_CXXLIBS += -lcurl

endif
