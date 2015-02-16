################################################################################
# Makefile for agenda project ##################################################
################################################################################
.PHONY: clean install

ifeq ($(MAKECMDGOALS),debug)
DO_DEBUG = true
endif

ifdef DO_DEBUG
COMPILE = g++ -g -c -Wall -pedantic-errors -Werror -Wextra -Wshadow -Wfatal-errors -Wno-unused-variable --std=c++11
LINK = g++
OBJDIR = dobj
PROGRAM = agenda-debug
else
COMPILE = g++ -c -Wall -pedantic-errors -Werror -Wextra -Wshadow -Wfatal-errors -Wno-unused-variable --std=c++11
LINK = g++ -O3 -s
OBJDIR = obj
PROGRAM = agenda
endif
LIBRARY =

BINSTREAM_H = binstream.h
DATETYPE_H = datetype.h $(BINSTREAM_H)
TASK_H = task.h $(DATETYPE_H)

OBJECTS = agenda.o task.o datetype.o binstream.o
OBJECTS := $(addprefix $(OBJDIR)/,$(OBJECTS))

all: $(OBJDIR) $(PROGRAM)
debug: $(OBJDIR) $(PROGRAM)

$(PROGRAM): $(OBJECTS)
	$(LINK) $(OBJECTS) $(LIBRARY) -o$(PROGRAM)

$(OBJDIR)/agenda.o: agenda.cpp $(TASK_H)
	$(COMPILE) agenda.cpp -o$(OBJDIR)/agenda.o
$(OBJDIR)/task.o: task.cpp $(TASK_H)
	$(COMPILE) task.cpp -o$(OBJDIR)/task.o
$(OBJDIR)/datetype.o: datetype.cpp $(DATETYPE_H)
	$(COMPILE) datetype.cpp -o$(OBJDIR)/datetype.o
$(OBJDIR)/binstream.o: binstream.cpp $(BINSTREAM_H)
	$(COMPILE) binstream.cpp -o$(OBJDIR)/binstream.o

$(OBJDIR):
	mkdir $(OBJDIR)

clean:
	@rm -rf obj/
	@rm -rf dobj/
	@rm -f agenda
	@rm -f agenda-debug

install:
	@cp --verbose agenda /usr/local/bin
