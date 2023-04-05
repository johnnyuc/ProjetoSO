# Author: António Silva 2020238160
# Author: Johnny Fernandes 2021190668
# LEI UC 2022-23 - Sistemas Operativos

# Compiler
CC = gcc

# Path to source and binary files
BINDIR = bin
SYS_DIR = src/home_iot
SENSOR_DIR = src/sensor
CONSOLE_DIR = src/user_console

# Flags
FLAGS_SYS = -Wall -I./$(SYS_DIR)/include
FLAGS_SENSOR = -Wall -I./$(SENSOR_DIR)/include
FLAGS_CONSOLE = -Wall -I./$(CONSOLE_DIR)/include

# Program names
PROG_SYS = home_iot
PROG_SENSOR = sensor
PROG_CONSOLE = user_console

# Object files
OBJS_SYS = $(SYS_DIR)/sys_manager.o 
OBJS_SENSOR = $(SENSOR_DIR)/sensor.o
OBJS_CONSOLE = $(CONSOLE_DIR)/user_console.o

# Default target
all: ${PROG_SYS} ${PROG_SENSOR} ${PROG_CONSOLE}

# Clean all object files and executables
clean:
	rm -f $(OBJS_SYS) $(OBJS_SENSOR) $(OBJS_CONSOLE) *~ $(PROG_SYS) $(PROG_SENSOR) $(PROG_CONSOLE)

# Render all executables
$(PROG_SYS): $(OBJS_SYS)
	$(CC) $(FLAGS_SYS) $(OBJS_SYS) -lm -o $(BINDIR)/$@

$(PROG_SENSOR): $(OBJS_SENSOR)
	$(CC) $(FLAGS_SENSOR) $(OBJS_SENSOR) -lm -o $(BINDIR)/$@

$(PROG_CONSOLE): $(OBJS_CONSOLE)
	$(CC) $(FLAGS_CONSOLE) $(OBJS_CONSOLE) -lm -o $(BINDIR)/$@

# Render all object files
# If there is a header file with the same name, compile it too
$(SYS_DIR)/%.o: $(SYS_DIR)/%.c $(SYS_DIR)/include/%.h
	$(CC) $(FLAGS_SYS) -c $< -o $@

$(SENSOR_DIR)/%.o: $(SENSOR_DIR)/%.c $(SENSOR_DIR)/include/%.h
	$(CC) $(FLAGS_SENSOR) -c $< -o $@

$(CONSOLE_DIR)/%.o: $(CONSOLE_DIR)/%.c $(CONSOLE_DIR)/include/%.h
	$(CC) $(FLAGS_CONSOLE) -c $< -o $@

