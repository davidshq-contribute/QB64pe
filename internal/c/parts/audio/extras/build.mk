
# foo_midi

FOO_MIDI_SRCS := \
	InstrumentBankManager.cpp \
	MIDIPlayer.cpp \
	OpalPlayer.cpp \
	PSPlayer.cpp \
	TSFPlayer.cpp

ifeq ($(OS),win)
	FOO_MIDI_SRCS += VSTiPlayer.cpp
endif

FOO_MIDI_OBJS := $(foreach src,$(FOO_MIDI_SRCS),$(call BUILD_OBJ,$(PATH_INTERNAL_C)/parts/audio/extras/foo_midi/$(src)))

$(BUILD_OBJ_DIR)/internal/c/parts/audio/extras/foo_midi/%.o: $(PATH_INTERNAL_C)/parts/audio/extras/foo_midi/%.cpp | $(BUILD_OBJ_DIR)
	$(call MKDIR_SAFE,$(dir $@))
	$(CXX) -O3 $(CXXFLAGS) -Wall $< -c -o $@

# hivelytracker

HIVELYTRACKER_SRCS := hvl_replay.c

HIVELYTRACKER_OBJS := $(call BUILD_OBJ,$(PATH_INTERNAL_C)/parts/audio/extras/hivelytracker/$(HIVELYTRACKER_SRCS))

$(BUILD_OBJ_DIR)/internal/c/parts/audio/extras/hivelytracker/%.o: $(PATH_INTERNAL_C)/parts/audio/extras/hivelytracker/%.c | $(BUILD_OBJ_DIR)
	$(call MKDIR_SAFE,$(dir $@))
	$(CC) -O3 $(CFLAGS) -Wall $< -c -o $@

#libmidi

LIBMIDI_SRCS := \
	MIDIContainer.cpp \
	MIDIProcessor.cpp \
	MIDIProcessorGMF.cpp \
	MIDIProcessorHMI.cpp \
	MIDIProcessorHMP.cpp \
	MIDIProcessorLDS.cpp \
	MIDIProcessorMDS.cpp \
	MIDIProcessorMUS.cpp \
	MIDIProcessorRCP.cpp \
	MIDIProcessorRIFF.cpp \
	MIDIProcessorSMF.cpp \
	MIDIProcessorXMI.cpp \
	Recomposer/CM6File.cpp \
	Recomposer/GDSFile.cpp \
	Recomposer/MIDIStream.cpp \
	Recomposer/RCP.cpp \
	Recomposer/RCPConverter.cpp \
	Recomposer/RunningNotes.cpp \
	Recomposer/Support.cpp

LIBMIDI_OBJS := $(foreach src,$(LIBMIDI_SRCS),$(call BUILD_OBJ,$(PATH_INTERNAL_C)/parts/audio/extras/libmidi/$(src)))

$(BUILD_OBJ_DIR)/internal/c/parts/audio/extras/libmidi/%.o: $(PATH_INTERNAL_C)/parts/audio/extras/libmidi/%.cpp | $(BUILD_OBJ_DIR)
	$(call MKDIR_SAFE,$(dir $@))
	$(CXX) -O3 $(CXXFLAGS) -Wall $< -c -o $@

# libxmp-lite

LIBXMP_SRCS := \
	common.c \
	control.c \
	dataio.c \
	effects.c \
	filetype.c \
	filter.c \
	flow.c \
	format.c \
	hio.c \
	it_load.c \
	itsex.c \
	lfo.c \
	load.c \
	load_helpers.c \
	md5.c \
	memio.c \
	misc.c \
	mix_all.c \
	mixer.c \
	mod_load.c \
	period.c \
	player.c \
	read_event.c \
	rng.c \
	s3m_load.c \
	sample.c \
	scan.c \
	smix.c \
	virtual.c \
	win32.c \
	xm_load.c

LIBXMP_OBJS := $(foreach src,$(LIBXMP_SRCS),$(call BUILD_OBJ,$(PATH_INTERNAL_C)/parts/audio/extras/libxmp-lite/$(src)))

$(BUILD_OBJ_DIR)/internal/c/parts/audio/extras/libxmp-lite/%.o: $(PATH_INTERNAL_C)/parts/audio/extras/libxmp-lite/%.c | $(BUILD_OBJ_DIR)
	$(call MKDIR_SAFE,$(dir $@))
	$(CC) -O3 $(CFLAGS) -Wall -DLIBXMP_CORE_PLAYER -DLIBXMP_STATIC $< -c -o $@

# primesynth

PRIMESYNTH_SRCS := primesynth.cpp

PRIMESYNTH_OBJS := $(call BUILD_OBJ,$(PATH_INTERNAL_C)/parts/audio/extras/primesynth/$(PRIMESYNTH_SRCS))

$(BUILD_OBJ_DIR)/internal/c/parts/audio/extras/primesynth/%.o: $(PATH_INTERNAL_C)/parts/audio/extras/primesynth/%.cpp | $(BUILD_OBJ_DIR)
	$(call MKDIR_SAFE,$(dir $@))
	$(CXX) -O3 $(CXXFLAGS) -Wall $< -c -o $@

# QOA

QOA_SRCS := qoa.c

QOA_OBJS := $(call BUILD_OBJ,$(PATH_INTERNAL_C)/parts/audio/extras/qoa/$(QOA_SRCS))

$(BUILD_OBJ_DIR)/internal/c/parts/audio/extras/qoa/%.o: $(PATH_INTERNAL_C)/parts/audio/extras/qoa/%.c | $(BUILD_OBJ_DIR)
	$(call MKDIR_SAFE,$(dir $@))
	$(CC) -O3 $(CFLAGS) -Wall $< -c -o $@

# radv2

OPAL_SRCS := opal.cpp

OPAL_OBJS := $(call BUILD_OBJ,$(PATH_INTERNAL_C)/parts/audio/extras/radv2/$(OPAL_SRCS))

$(BUILD_OBJ_DIR)/internal/c/parts/audio/extras/radv2/%.o: $(PATH_INTERNAL_C)/parts/audio/extras/radv2/%.cpp | $(BUILD_OBJ_DIR)
	$(call MKDIR_SAFE,$(dir $@))
	$(CXX) -O3 $(CXXFLAGS) -Wall $< -c -o $@

# stb_vorbis

STB_VORBIS_SRCS := stb_vorbis.c

STB_VORBIS_OBJS := $(call BUILD_OBJ,$(PATH_INTERNAL_C)/parts/audio/extras/stb/$(STB_VORBIS_SRCS))

$(BUILD_OBJ_DIR)/internal/c/parts/audio/extras/stb/%.o: $(PATH_INTERNAL_C)/parts/audio/extras/stb/%.c | $(BUILD_OBJ_DIR)
	$(call MKDIR_SAFE,$(dir $@))
	$(CC) -O3 $(CFLAGS) -Wall $< -c -o $@

# TinySoundFont

TINYSOUNDFONT_SRCS := tsf.c

TINYSOUNDFONT_OBJS := $(call BUILD_OBJ,$(PATH_INTERNAL_C)/parts/audio/extras/tinysoundfont/$(TINYSOUNDFONT_SRCS))

$(BUILD_OBJ_DIR)/internal/c/parts/audio/extras/tinysoundfont/%.o: $(PATH_INTERNAL_C)/parts/audio/extras/tinysoundfont/%.c | $(BUILD_OBJ_DIR)
	$(call MKDIR_SAFE,$(dir $@))
	$(CC) -O3 $(CFLAGS) -Wall $< -c -o $@

# ymfmidi

YMFMIDI_SRCS := \
	patches.cpp \
	player.cpp

YMFMIDI_OBJS := $(foreach src,$(YMFMIDI_SRCS),$(call BUILD_OBJ,$(PATH_INTERNAL_C)/parts/audio/extras/ymfmidi/$(src)))

$(BUILD_OBJ_DIR)/internal/c/parts/audio/extras/ymfmidi/%.o: $(PATH_INTERNAL_C)/parts/audio/extras/ymfmidi/%.cpp | $(BUILD_OBJ_DIR)
	$(call MKDIR_SAFE,$(dir $@))
	$(CXX) -O3 $(CXXFLAGS) -Wall $< -c -o $@

# ma_vtables

MA_VTABLES_SRCS := \
	hively_ma_vtable.cpp \
	midi_ma_vtable.cpp \
	mod_ma_vtable.cpp \
	qoa_ma_vtable.cpp \
	radv2_ma_vtable.cpp

MA_VTABLES_OBJS := $(foreach src,$(MA_VTABLES_SRCS),$(call BUILD_OBJ,$(PATH_INTERNAL_C)/parts/audio/extras/$(src)))

$(BUILD_OBJ_DIR)/internal/c/parts/audio/extras/%.o: $(PATH_INTERNAL_C)/parts/audio/extras/%.cpp | $(BUILD_OBJ_DIR)
	$(call MKDIR_SAFE,$(dir $@))
	$(CXX) -O3 $(CXXFLAGS) -Wall $< -c -o $@

CLEAN_LIST += $(FOO_MIDI_OBJS) $(HIVELYTRACKER_OBJS) $(LIBMIDI_OBJS) $(LIBXMP_OBJS) $(PRIMESYNTH_OBJS) $(QOA_OBJS) $(OPAL_OBJS) $(STB_VORBIS_OBJS) $(TINYSOUNDFONT_OBJS) $(YMFMIDI_OBJS) $(MA_VTABLES_OBJS)
