CXX ?= g++
CPPFLAGS ?=
CXXFLAGS ?= -O2 -std=c++20 -Wall -Wextra -Wpedantic
LDLIBS := -l:libSDL2-2.0.so.0 -l:libcairo.so.2 -l:libGL.so.1 -l:libfontconfig.so.1

SOURCES := $(wildcard src/*.cpp)
OBJECTS := $(patsubst src/%.cpp,.build/%.o,$(SOURCES))

.PHONY: all test clean
all: biblioteca

biblioteca: $(OBJECTS)
	$(CXX) $(OBJECTS) -o $@ $(LDFLAGS) $(LDLIBS)

.build/%.o: src/%.cpp
	@mkdir -p .build
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -MMD -MP -c $< -o $@

-include $(OBJECTS:.o=.d)

test: biblioteca
	./biblioteca --self-test

clean:
	rm -rf .build biblioteca
