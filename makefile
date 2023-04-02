# Author: António Silva e Johnny Fernandes
# LEI UC 2022-23 - Sistemas Operativos

# Compiler and flags
CC = gcc
FLAGS = -Wall -I./include

# Path to source and binary files
SRCDIR = src
BINDIR = bin

# Program names
PROG_SYS = sys_manager
PROG_SENSOR = sensor
PROG_CONSOLE = user_console

# Object files
OBJS_SYS = $(SRCDIR)/sys_manager.o
OBJS_SENSOR = $(SRCDIR)/sensor.o
OBJS_CONSOLE = $(SRCDIR)/user_console.o

# Default target
all: ${PROG_SYS} ${PROG_SENSOR} ${PROG_CONSOLE}

# Clean all object files and executables
clean:
	rm -f $(OBJS_SYS) $(OBJS_SENSOR) $(OBJS_CONSOLE) *~ $(PROG_SYS) $(PROG_SENSOR) $(PROG_CONSOLE)

# Render all executables
$(PROG_SYS): $(OBJS_SYS)
	$(CC) $(FLAGS) $(OBJS_SYS) -lm -o $(BINDIR)/$@

$(PROG_SENSOR): $(OBJS_SENSOR)
	$(CC) $(FLAGS) $(OBJS_SENSOR) -lm -o $(BINDIR)/$@

$(PROG_CONSOLE): $(OBJS_CONSOLE)
	$(CC) $(FLAGS) $(OBJS_CONSOLE) -lm -o $(BINDIR)/$@

# Render all object files
# If there is a header file with the same name, compile it too
.c.o:
	$(CC) $(FLAGS) -c -o $@ $<
	@if [ -f $(SRCDIR)/$*.h ]; then \
		$(CC) $(FLAGS) -c -o $@ $(SRCDIR)/$*.h; \
	fi