TARGET = play_icfp2015
PROFILE = profile
VISUALIZER = visualizer

CXX = g++

OVERALL_OPTIONS = -pipe
LANGUAGE_OPTIONS = -std=c++11
WARNING_OPTIONS = -Wall -Wextra -Woverloaded-virtual #-Werror #-fcolor-diagnostics
OPTIMIZATION_OPTIONS = -O3 -fno-omit-frame-pointer -march=native -mtune=native
CODE_GENERATION_OPTIONS = -fPIC -pthread
PREPROCESSOR_OPTIONS = -MMD -MP
DEBUGGING_OPTIONS = -gdwarf-3 -fsanitize=address
INCLUDE_OPTIONS = -Iextlib
CXXFLAGS = $(OVERALL_OPTIONS) $(LANGUAGE_OPTIONS) $(WARNING_OPTIONS) $(OPTIMIZATION_OPTIONS) \
           $(CODE_GENERATION_OPTIONS) $(PREPROCESSOR_OPTIONS) $(DEBUGGING_OPTIONS) $(INCLUDE_OPTIONS)

LDFLAGS = -pthread -fsanitize=address
LIBS = -lm -lpthread

SOURCES = $(wildcard src/*.cc)
OBJECTS = $(patsubst src/%.cc, obj/main/%.o, $(SOURCES))
DEPENDS = $(patsubst %.o, %.d, $(OBJECTS))

PROFILE_OBJECTS = $(patsubst src/%.cc, obj/profile/%.o, $(SOURCES))
PROFILE_DEPENDS = $(patsubst %.o, %.d, $(PROFILE_OBJECTS))

TEST_SOURCES = $(wildcard test/*.cc)
TEST_OBJECTS = $(patsubst test/%.cc, obj/test/%.o, $(TEST_SOURCES))
TEST_DEPENDS = $(patsubst %.o, %.d, $(TEST_OBJECTS))
TESTS = $(patsubst test/%.cc, obj/test/%.exe, $(TEST_SOURCES))

VISUALIZER_SOURCES = $(wildcard src_visualizer/*.cc)
VISUALIZER_OBJECTS = $(patsubst src_visualizer/%.cc, obj/visualizer/%.o, $(VISUALIZER_SOURCES))
VISUALIZER_DEPENDS = $(patsubst %.o, %.d, $(VISUALIZER_OBJECTS))
#VISUALIZER_CPPFLAGS = -Isrc -I/usr/include/SDL2 -D_REENTRANT
#VISUALIZER_LIBS = -L/usr/lib/x86_64-linux-gnu -lSDL2 -lSDL2_image

GTEST_DIR = extlib/gtest
GTEST_OBJ_DIR = obj/gtest
GTEST_HEADERS = $(GTEST_DIR)/include/gtest/*.h $(GTEST_DIR)/include/gtest/internal/*.h
GTEST_SOURCES = $(GTEST_DIR)/src/*.cc $(GTEST_DIR)/src/*.h $(GTEST_HEADERS)




#==============================================================================
# Build rules for TARGET
#==============================================================================

$(TARGET): $(OBJECTS)
	$(CXX) $(LDFLAGS) -o $@ $^ $(LIBS)

obj/main/%.o: src/%.cc
	@mkdir -p obj/main
	$(CXX) $(CXXFLAGS) -o $@ -c $<

-include $(DEPENDS)

#==============================================================================
# Build rules for POFILE
#==============================================================================

$(PROFILE): $(PROFILE_OBJECTS)
	$(CXX) $(LDFLAGS) -pg -o $@ $^ $(LIBS)

obj/profile/%.o: src/%.cc
	@mkdir -p obj/profile
	$(CXX) $(CXXFLAGS) -pg -o $@ -c $<

-include $(PROFILE_DEPENDS)


#==============================================================================
# Build rules for VISUALIZER
#==============================================================================

$(VISUALIZER): $(filter-out obj/main/main.o, $(OBJECTS)) $(VISUALIZER_OBJECTS)
	$(CXX) $(LDFLAGS) -o $@ $^ $(LIBS) $(shell sdl2-config --libs) -lSDL2_image

obj/visualizer/%.o: src_visualizer/%.cc
	@mkdir -p obj/visualizer
	$(CXX) $(CXXFLAGS) -Isrc $(shell sdl2-config --cflags) -o $@ -c $<

-include $(VISUALIZER_DEPENDS)


#==============================================================================
# Build rules for TESTS
#==============================================================================

test: $(TESTS)

obj/test/%.exe: obj/test/%.o $(filter-out obj/main/main.o, $(OBJECTS)) $(GTEST_OBJ_DIR)/gtest_main.a
	$(CXX) $(LDFLAGS) $^ -o $@ $(LIBS)

obj/test/%.o: test/%.cc
	@mkdir -p obj/test
	$(CXX) -isystem $(GTEST_DIR)/include -Isrc $(CXXFLAGS) -o $@ -c $<

-include $(TEST_DEPENDS)


#==============================================================================
# Build rules for Google Test
#==============================================================================

$(GTEST_OBJ_DIR)/gtest-all.o: $(GTEST_SOURCES)
	@mkdir -p $(GTEST_OBJ_DIR)
	$(CXX) -isystem $(GTEST_DIR) -isystem $(GTEST_DIR)/include $(CXXFLAGS) -o $@ -c $(GTEST_DIR)/src/gtest-all.cc

$(GTEST_OBJ_DIR)/gtest_main.o: $(GTEST_SOURCES)
	@mkdir -p $(GTEST_OBJ_DIR)
	$(CXX) -isystem $(GTEST_DIR) -isystem $(GTEST_DIR)/include $(CXXFLAGS) -o $@ -c $(GTEST_DIR)/src/gtest_main.cc

$(GTEST_OBJ_DIR)/gtest.a: $(GTEST_OBJ_DIR)/gtest-all.o
	$(AR) $(ARFLAGS) $@ $^

$(GTEST_OBJ_DIR)/gtest_main.a: $(GTEST_OBJ_DIR)/gtest-all.o $(GTEST_OBJ_DIR)/gtest_main.o
	$(AR) $(ARFLAGS) $@ $^


#==============================================================================
# Misc
#==============================================================================

all: play_icfp2015 visualizer test

clean:
	rm -rf obj/ $(TARGET) $(PROFILE) $(VISUALIZER)

.PHONY: clean test

# do not delete intermediate files
.SECONDARY:
