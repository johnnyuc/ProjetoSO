# Author: Johnny Fernandes 2021190668
# LEI UC 2022-23 - Sistemas Operativos

# Compiler:
CC = gcc

# Compiler flags:
CFLAGS = -Wall -Werror -Wextra -pedantic -I. # Making sure either is flawless or breaks
LDFLAGS = -pthread

# Source folders:
SRCDIR = src
LOGDIR = log

# HOME_IOT files:
HOME_IOT_SRCS = $(wildcard $(SRCDIR)/home_iot/*.c)
HOME_IOT_OBJS = $(HOME_IOT_SRCS:.c=.o)
HOME_IOT_TARGET = bin/home_iot

# SENSOR files:
SENSOR_SRCS = $(wildcard $(SRCDIR)/sensor/*.c)
SENSOR_OBJS = $(SENSOR_SRCS:.c=.o)
SENSOR_TARGET = bin/sensor

# USER_CONSOLE files:
USER_CONSOLE_SRCS = $(wildcard $(SRCDIR)/user_console/*.c)
USER_CONSOLE_OBJS = $(USER_CONSOLE_SRCS:.c=.o)
USER_CONSOLE_TARGET = bin/user_console

# Default target:
all: $(HOME_IOT_TARGET) $(SENSOR_TARGET) $(USER_CONSOLE_TARGET)

# Linking the object files
# May or may not use LDFLAGS, depending on SENSOR and CONSOLE implementation
$(HOME_IOT_TARGET): $(HOME_IOT_OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(HOME_IOT_OBJS) -o $(HOME_IOT_TARGET)

$(SENSOR_TARGET): $(SENSOR_OBJS)
	$(CC) $(CFLAGS) $(SENSOR_OBJS) -o $(SENSOR_TARGET)

$(USER_CONSOLE_TARGET): $(USER_CONSOLE_OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(USER_CONSOLE_OBJS) -o $(USER_CONSOLE_TARGET)

# Compiling the source files
%.o: %.c %.h
	$(CC) $(CFLAGS) -c $< -o $@

# Cleaning the project including log files
clean:
	rm -f $(HOME_IOT_OBJS) $(HOME_IOT_TARGET) $(SENSOR_OBJS) $(SENSOR_TARGET) $(USER_CONSOLE_OBJS) $(USER_CONSOLE_TARGET) $(LOGDIR)/*.log *_PIPE