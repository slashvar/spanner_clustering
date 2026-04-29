CXX      = c++
CXXFLAGS = -std=c++20 -Wall -Wextra -g -I../src
TESTS    = test_point_set test_union_find test_tree test_wspd test_integration

all: $(TESTS)

UNAME_S := $(shell uname -s)
UNAME_M := $(shell uname -m)

test_%: test_%.cc test_harness.hh ../src/*.hh
	$(CXX) $(CXXFLAGS) -o $@ $<
ifeq ($(UNAME_S),Darwin)
ifeq ($(UNAME_M),arm64)
	codesign -s - $@
endif
endif

test: $(TESTS)
	@for t in $(TESTS); do echo "=== $$t ===" && ./$$t && echo || exit 1; done

clean:
	rm -rf $(TESTS) *.dSYM

.PHONY: all test clean
