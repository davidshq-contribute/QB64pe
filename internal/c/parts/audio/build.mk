
AUDIO_STUB_SRCS := stub_audio.cpp

AUDIO_STUB_OBJS := $(call BUILD_OBJ,$(PATH_INTERNAL_C)/parts/audio/stub_audio.cpp)

AUDIO_SRCS := audio.cpp

AUDIO_OBJS := $(call BUILD_OBJ,$(PATH_INTERNAL_C)/parts/audio/audio.cpp)

MINIAUDIO_SRCS := miniaudio/miniaudio.c

MINIAUDIO_OBJS := $(call BUILD_OBJ,$(PATH_INTERNAL_C)/parts/audio/miniaudio/miniaudio.c)

# DEPENDENCY_CONSOLE_ONLY is added here to keep these .cpp files from including
# the FreeGLUT headers via `libqb.h`. Ideally this is fixed properly in the future.
$(BUILD_OBJ_DIR)/internal/c/parts/audio/%.o: $(PATH_INTERNAL_C)/parts/audio/%.cpp | $(BUILD_OBJ_DIR)
	$(call MKDIR_SAFE,$(dir $@))
	$(CXX) -O3 $(CXXFLAGS) -DDEPENDENCY_CONSOLE_ONLY -Wall $< -c -o $@

$(BUILD_OBJ_DIR)/internal/c/parts/audio/%.o: $(PATH_INTERNAL_C)/parts/audio/%.c | $(BUILD_OBJ_DIR)
	$(call MKDIR_SAFE,$(dir $@))
	$(CC) -O3 $(CFLAGS) -Wall $< -c -o $@

AUDIO_LIB := $(call BUILD_LIB,audio)

$(AUDIO_LIB): $(AUDIO_OBJS) $(MINIAUDIO_OBJS) $(FOO_MIDI_OBJS) $(HIVELYTRACKER_OBJS) $(LIBMIDI_OBJS) $(LIBXMP_OBJS) $(PRIMESYNTH_OBJS) $(QOA_OBJS) $(OPAL_OBJS) $(STB_VORBIS_OBJS) $(TINYSOUNDFONT_OBJS) $(YMFMIDI_OBJS) $(MA_VTABLES_OBJS) | $(BUILD_LIB_DIR)
	$(call MKDIR_SAFE,$(dir $@))
	$(AR) rcs $@  $(AUDIO_OBJS) $(MINIAUDIO_OBJS) $(FOO_MIDI_OBJS) $(HIVELYTRACKER_OBJS) $(LIBMIDI_OBJS) $(LIBXMP_OBJS) $(PRIMESYNTH_OBJS) $(QOA_OBJS) $(OPAL_OBJS) $(STB_VORBIS_OBJS) $(TINYSOUNDFONT_OBJS) $(YMFMIDI_OBJS) $(MA_VTABLES_OBJS)

CLEAN_LIST += $(AUDIO_STUB_OBJS) $(AUDIO_OBJS) $(MINIAUDIO_OBJS) $(AUDIO_LIB)
