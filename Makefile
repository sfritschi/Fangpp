CXX=clang++
CXXFLAGS=-Wall -Wextra -Wconversion -pedantic -ggdb3 -gdwarf-4 -std=c++20
PUGI_CXXFLAGS=-Wall -Wextra -pedantic -std=c++20 -O3

TARGET=fangpp
.PHONY: all, clean
all=$(TARGET)
#LINK_GL=-lGL -lglut -lGLEW -lm
#LINK_FT=-lfreetype -lpng -lbz2 -lz

EXTDIR=ext
PUGIXML=$(EXTDIR)/pugixml/src
INCLUDE=-Iinclude
INCLUDE+=-I$(PUGIXML)
SRCDIR=src
OBJDIR=bin

OBJ=$(patsubst $(SRCDIR)/%.cpp,$(OBJDIR)/%.o,$(wildcard $(SRCDIR)/*.cpp))
OBJ+=$(patsubst $(PUGIXML)/%.cpp,$(OBJDIR)/%.o,$(wildcard $(PUGIXML)/*.cpp))

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp | $(OBJDIR)
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c $^ -o $@

$(OBJDIR)/%.o: $(PUGIXML)/%.cpp | $(OBJDIR)
	$(CXX) $(PUGI_CXXFLAGS) $(PUGI_INCLUDE) -c $^ -o $@
	
$(TARGET): $(OBJ)
	$(CXX) $^ -o $@

$(OBJDIR):
	mkdir -p $@

clean:
	$(RM) $(TARGET)
	$(RM) -r $(OBJDIR)
