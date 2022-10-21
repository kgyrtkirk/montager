
# enable/disable debug mode
DEBUG ?= 0
ifeq ($(DEBUG), 1)
    CXXFLAGS += -DDEBUG=1
else
    CXXFLAGS += -DDEBUG=0
endif


# set standard values, if not set by default
CXX ?= g++
CXXFLAGS += -O3 -Wall -fpermissive


# project-specific flags
EXTRA_CXXFLAGS += $(shell gimptool-2.0 --cflags)
LDFLAGS += $(shell gimptool-2.0 --libs) -lstdc++

# project data
PLUGIN = montager
SOURCES = $(wildcard src/*.cpp)
HEADERS = $(wildcard src/*.h)

# END CONFIG ##################################################################

.PHONY: all install userinstall clean uninstall useruninstall

all: $(PLUGIN)

OBJECTS = $(subst .cpp,.o,$(SOURCES))

$(PLUGIN): $(OBJECTS)
	echo src:$(SOURCES)
	$(CXX) -o $@ $^ $(LDFLAGS)

%.o: %.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) $(EXTRA_CXXFLAGS) -c -o $@ $*.cpp

install: $(PLUGIN)
	@gimptool-2.0 --install-admin-bin $^

userinstall: $(PLUGIN)
	@gimptool-2.0 --install-bin $^

uninstall:
	@gimptool-2.0 --uninstall-admin-bin $(PLUGIN)

useruninstall:
	@gimptool-2.0 --uninstall-bin $(PLUGIN)

clean:
	rm -f src/*.o $(PLUGIN)

debug:
	$(MAKE) $(MAKEFILE) DEBUG="-g -g3 -gdwarf-2 -D DEBUG"
