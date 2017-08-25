THIRD_PATH = $(CURDIR)/third
GTEST_DIR = $(THIRD_PATH)/gtest

PINK_PATH=$(THIRD_PATH)/pink
PINK_INCLUDE_DIR=$(PINK_PATH)
PINK_LIBRARY=$(PINK_PATH)/pink/lib/libpink.a

SLASH_PATH=$(THIRD_PATH)/slash
SLASH_INCLUDE_DIR=$(SLASH_PATH)
SLASH_LIBRARY=$(SLASH_PATH)/slash/lib/libslash.a

LIBZP_PATH=$(THIRD_PATH)/zeppelin-client/libzp
LIBZP_INCLUDE_DIR=$(LIBZP_PATH)
LIBZP_LIBRARY=$(LIBZP_PATH)/libzp/lib/libzp.a

DEP_LIBS=$(LIBZP_LIBRARY) $(PINK_LIBRARY) $(SLASH_LIBRARY)


LDFLAGS = $(DEP_LIBS) -lpthread -lprotobuf
EXTRA_CXXFLAGS = -I$(PINK_INCLUDE_DIR) -I$(SLASH_INCLUDE_DIR) -I$(LIBZP_INCLUDE_DIR)

# Flags passed to the preprocessor.
# Set Google Test and Google Mock's header directories as system
# directories, such that the compiler doesn't generate warnings in
# these headers.
CPPFLAGS += -isystem $(GTEST_DIR)/include -isystem $(GMOCK_DIR)/include

# Flags passed to the C++ compiler.
CXXFLAGS += -g -Wall -Wextra -pthread -std=c++11 $(EXTRA_CXXFLAGS)

# All tests produced by this Makefile.  Remember to add new tests you
# created to the list.
TESTS = zp_basis_tests \
				zp_sync_tests \

# All Google Test headers.  Usually you shouldn't change this
# definition.
GTEST_HEADERS = $(GTEST_DIR)/include/gtest/*.h \
                $(GTEST_DIR)/include/gtest/internal/*.h

# House-keeping build targets.

.PHONY: clean all distclean

all : $(TESTS)

clean :
	rm -f $(TESTS) *.o

distclean:
	@echo "clean gtest"
	@rm -rf $(GTEST_ALL_O)
	make -C $(PINK_PATH)/pink clean
	make -C $(SLASH_PATH)/slash clean
	make -C $(LIBZP_PATH)/libzp clean

GTEST_SRCS_ = $(GTEST_DIR)/src/*.cc $(GTEST_DIR)/src/*.h $(GTEST_HEADERS)
GTEST_ALL=$(GTEST_DIR)/src/gtest-all.cc
GTEST_ALL_O= $(GTEST_ALL:.cc=.o)

$(GTEST_ALL_O) : $(GTEST_ALL) $(GTEST_SRCS_)
	$(CXX) $(CPPFLAGS) -I$(GTEST_DIR) $(CXXFLAGS) -c $< -o $@

$(SLASH_LIBRARY):
	@echo "make slash"
	@make -C $(SLASH_PATH)/slash

$(PINK_LIBRARY): $(SLASH_LIBRARY)
	@echo "make pink"
	@make -C $(PINK_PATH)/pink SLASH_PATH=$(SLASH_PATH)

$(LIBZP_LIBRARY): $(PINK_LIBRARY) $(SLASH_LIBRARY)
	@echo "make libzp"
	@make -C $(LIBZP_PATH)/libzp PINK_PATH=$(PINK_PATH) SLASH_PATH=$(SLASH_PATH)

# Builds zeppelin tests.

# Objects
zp_basis_tests.o : zp_basis_tests.cc zp_tests_env.h
	$(CXX) -o $@ $(CPPFLAGS) $(CXXFLAGS) -c $<

zp_sync_tests.o : zp_sync_tests.cc zp_tests_env.h
	$(CXX) -o $@ $(CPPFLAGS) $(CXXFLAGS) -c $<

zp_tests_env.o : zp_tests_env.cc zp_tests_env.h
	$(CXX) -o $@ $(CPPFLAGS) $(CXXFLAGS) -c $<

# Binaries
zp_basis_tests: zp_basis_tests.o zp_tests_env.o $(DEP_LIBS) $(GTEST_ALL_O)
	$(CXX) $^ $(LDFLAGS) -o $@

zp_sync_tests: zp_sync_tests.o zp_tests_env.o $(DEP_LIBS) $(GTEST_ALL_O)
	$(CXX) $^ $(LDFLAGS) -o $@
