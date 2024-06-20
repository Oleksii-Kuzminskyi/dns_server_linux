TARGET = ./bin/dns_server

SRCDIR = ./sources/
INCDIR = ./headers/
OBJDIR = ./obj/

SOURCES = $(wildcard $(SRCDIR)*.cpp)
OBJECTS = $(patsubst $(SRCDIR)%.cpp, $(OBJDIR)%.o, $(SOURCES))

CXX = g++
override CFLAGS += -std=c++14 -Wall -Wextra
override LDFLAGS += -lPocoNet -lsqlite3
INCLUDES += -I$(INCDIR)


all: $(OBJECTS)
	$(CXX) $(LDFLAGS) $(OBJECTS) -o $(TARGET)


$(OBJDIR)%.o : $(SRCDIR)%.cpp
	$(CXX) $(INCLUDES) $(DEFINES) $(CFLAGS) -c $< -o $@


clean :
	rm $(TARGET) $(OBJDIR)*.o













