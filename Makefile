CXX=clang++
CXXFLAGS=-Wall -Wextra -Wpedantic -std=c++20 -pthread -ggdb3
PUGI_CXXFLAGS=-Wall -Wextra -pedantic -std=c++20 -O3

LIBFLAGS=-lfmod -lOpenGL -lGLEW -lglfw
LIBFLAGS+=-L/usr/local/lib -lfreetype -lpng -lbz2 -lz

EXTDIR=ext
PUGIXML=$(EXTDIR)/pugixml/src
INCLUDE=-Iinclude
INCLUDE+=-I$(PUGIXML)
FMOD_CORE_DIR=$(USER_LIB_DIR)/fmod/api/core
INCLUDE+=-I$(FMOD_CORE_DIR)/inc
INCLUDE+=-I/usr/local/include/freetype2

SRCDIR=src
OBJDIR=bin

OBJ=$(patsubst $(SRCDIR)/%.cpp,$(OBJDIR)/%.o,$(wildcard $(SRCDIR)/*.cpp))
OBJ+=$(patsubst $(PUGIXML)/%.cpp,$(OBJDIR)/%.o,$(wildcard $(PUGIXML)/*.cpp))
	
TARGET=fangpp
.PHONY: all, clean
all: $(TARGET)

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp | $(OBJDIR)
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c $^ -o $@

$(OBJDIR)/%.o: $(PUGIXML)/%.cpp | $(OBJDIR)
	$(CXX) $(PUGI_CXXFLAGS) $(PUGI_INCLUDE) -c $^ -o $@
	
$(TARGET): $(OBJ)
	$(CXX) $^ -o $@ $(LIBFLAGS)

$(OBJDIR):
	mkdir -p $@
	
clean:
	$(RM) -r $(OBJDIR)
	$(RM) $(TARGET)
