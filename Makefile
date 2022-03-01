CXXFLAGS=-g0 -O3 -flto -march=native -fomit-frame-pointer 
# CXXFLAGS=-g -O2
# CXXFLAGS=-g -O0

CXXFLAGS+=-D_DESKTOP
CXXFLAGS+=-std=c++17 -iquote ./src/ -fopenmp -Wall -Werror -Wextra


CXX ?= g++

LD_FLAGS=-lm

SOURCES=$(shell find src | grep -v wasm | grep '\.cpp$$')
OBJECTS=$(patsubst %.cpp, %.o, $(SOURCES))
BOTLE ?= ./build/native/botle
SHELL=/bin/bash
ALL_DEPS=$(shell find .deps -type f 2> /dev/null)

.PHONY: all build clean wasm botle default

default: all

all: botle wasm

wasm:
	$(MAKE) -f Makefile.wasm

botle: $(BOTLE)

$(BOTLE): $(OBJECTS)
	@echo "Linking $@"
	@mkdir -p `dirname $@`
	@$(CXX) $(OBJECTS) -o $@ $(CXXFLAGS) $(LD_FLAGS)

-include $(ALL_DEPS)

%.o: %.cpp Makefile
	@mkdir -p `dirname .deps/$<.dep`
	@echo "Building $@"
	@$(CXX) $(CXXFLAGS) -MMD -MF .deps/$<.dep  -c $< -o $@

format:
	clang-format -i $(shell find . | grep hpp$$) $(shell find . | grep cpp$$)

clean:
	rm -rf $(BOTLE) $(OBJECTS) .deps
	$(MAKE) -f Makefile.wasm clean
