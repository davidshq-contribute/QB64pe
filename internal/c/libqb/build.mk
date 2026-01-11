
libqb-objs-y += $(call BUILD_OBJ,$(PATH_LIBQB)/src/threading.cpp)
libqb-objs-y += $(call BUILD_OBJ,$(PATH_LIBQB)/src/buffer.cpp)
libqb-objs-y += $(call BUILD_OBJ,$(PATH_LIBQB)/src/bitops.cpp)
libqb-objs-y += $(call BUILD_OBJ,$(PATH_LIBQB)/src/command.cpp)
libqb-objs-y += $(call BUILD_OBJ,$(PATH_LIBQB)/src/environ.cpp)
libqb-objs-y += $(call BUILD_OBJ,$(PATH_LIBQB)/src/file-fields.cpp)
libqb-objs-y += $(call BUILD_OBJ,$(PATH_LIBQB)/src/filepath.cpp)
libqb-objs-y += $(call BUILD_OBJ,$(PATH_LIBQB)/src/filesystem.cpp)
libqb-objs-y += $(call BUILD_OBJ,$(PATH_LIBQB)/src/datetime.cpp)
libqb-objs-y += $(call BUILD_OBJ,$(PATH_LIBQB)/src/error_handle.cpp)
libqb-objs-y += $(call BUILD_OBJ,$(PATH_LIBQB)/src/gfs.cpp)
libqb-objs-y += $(call BUILD_OBJ,$(PATH_LIBQB)/src/qblist.cpp)
libqb-objs-y += $(call BUILD_OBJ,$(PATH_LIBQB)/src/hexoctbin.cpp)
libqb-objs-y += $(call BUILD_OBJ,$(PATH_LIBQB)/src/mem.cpp)
libqb-objs-y += $(call BUILD_OBJ,$(PATH_LIBQB)/src/shell.cpp)
libqb-objs-y += $(call BUILD_OBJ,$(PATH_LIBQB)/src/qbs.cpp)
libqb-objs-y += $(call BUILD_OBJ,$(PATH_LIBQB)/src/qbs_str.cpp)
libqb-objs-y += $(call BUILD_OBJ,$(PATH_LIBQB)/src/qbs__tostr.cpp)
libqb-objs-y += $(call BUILD_OBJ,$(PATH_LIBQB)/src/qbs_cmem.cpp)
libqb-objs-y += $(call BUILD_OBJ,$(PATH_LIBQB)/src/qbs_mk_cv.cpp)
libqb-objs-y += $(call BUILD_OBJ,$(PATH_LIBQB)/src/qbs_val.cpp)
libqb-objs-y += $(call BUILD_OBJ,$(PATH_LIBQB)/src/string_functions.cpp)
libqb-objs-y += $(call BUILD_OBJ,$(PATH_LIBQB)/src/graphics.cpp)

libqb-objs-y += $(call BUILD_OBJ,$(PATH_LIBQB)/src/logging/logging.cpp)
libqb-objs-y += $(call BUILD_OBJ,$(PATH_LIBQB)/src/logging/qb64pe_symbol.cpp)
libqb-objs-y += $(call BUILD_OBJ,$(PATH_LIBQB)/src/logging/stacktrace.cpp)
libqb-objs-y += $(call BUILD_OBJ,$(PATH_LIBQB)/src/logging/handlers/fp_handler.cpp)

# Windows MinGW symbol resolution
libqb-objs-$(win) += $(call BUILD_OBJ,$(PATH_LIBQB)/src/logging/mingw/file.cpp)
libqb-objs-$(win) += $(call BUILD_OBJ,$(PATH_LIBQB)/src/logging/mingw/pe.cpp)
libqb-objs-$(win) += $(call BUILD_OBJ,$(PATH_LIBQB)/src/logging/mingw/pe_symtab.cpp)
libqb-objs-$(win) += $(call BUILD_OBJ,$(PATH_LIBQB)/src/logging/mingw/symbol.cpp)

# Unix symbol resolution
libqb-objs-$(unix) += $(call BUILD_OBJ,$(PATH_LIBQB)/src/logging/unix/symbol.cpp)

libqb-objs-$(DEP_HTTP) += $(call BUILD_OBJ,$(PATH_LIBQB)/src/http.cpp)
libqb-objs-y$(DEP_HTTP) += $(call BUILD_OBJ,$(PATH_LIBQB)/src/http-stub.cpp)

libqb-objs-y += $(call BUILD_OBJ,$(PATH_LIBQB)/src/threading-$(PLATFORM).cpp)

libqb-objs-y$(DEP_CONSOLE_ONLY) += $(call BUILD_OBJ,$(PATH_LIBQB)/src/glut-main-thread.cpp)
libqb-objs-y$(DEP_CONSOLE_ONLY) += $(call BUILD_OBJ,$(PATH_LIBQB)/src/glut-message.cpp)
libqb-objs-y$(DEP_CONSOLE_ONLY) += $(call BUILD_OBJ,$(PATH_LIBQB)/src/glut-msg-queue.cpp)

libqb-objs-$(DEP_CONSOLE_ONLY) += $(call BUILD_OBJ,$(PATH_LIBQB)/src/console-only-main-thread.cpp)

ifeq ($(OS),osx)
libqb-objs-y$(DEP_CONSOLE_ONLY) += $(call BUILD_OBJ,$(PATH_LIBQB)/src/mac-key-monitor.mm) $(call BUILD_OBJ,$(PATH_LIBQB)/src/mac-mouse-support.mm)
endif

# Pattern rules for libqb source files
# Transform source path to build path: internal/c/libqb/src/file.cpp -> $(BUILD_OBJ_DIR)/internal/c/libqb/src/file.o
# Handle both ./internal and internal paths by normalizing
$(BUILD_OBJ_DIR)/internal/c/libqb/src/%.o: $(PATH_INTERNAL_C)/libqb/src/%.cpp | $(BUILD_OBJ_DIR)
	$(call MKDIR_SAFE,$(dir $@))
	$(CXX) -O2 $(CXXFLAGS) -Wall -Wextra $< -c -o $@

ifeq ($(OS),osx)
$(BUILD_OBJ_DIR)/internal/c/libqb/src/%.o: $(PATH_INTERNAL_C)/libqb/src/%.mm | $(BUILD_OBJ_DIR)
	$(call MKDIR_SAFE,$(dir $@))
	$(CXX) -O2 $(CXXFLAGS) -Wall -Wextra $< -c -o $@
endif

CLEAN_LIST += $(libqb-objs-y) $(libqb-objs-yy) $(libqb-objs-)
