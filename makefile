# Makefile provisório, para já dá para o gasto
# Considerar que PROG deverá ser um nome distinto
# para cada programa que se pretende compilar

CC = gcc
FLAGS = -Wall
PROG = program
#OBJS = example
OBJS = sys_manager.o

all: ${PROG}

clean:
	rm ${OBJS} *~ ${PROG}
 
${PROG}: ${OBJS}
	${CC} ${FLAGS} ${OBJS} -lm -o $@

.c.o:
	${CC} ${FLAGS} $< -c -o $@

##########################
#example.o: example.h example.c
sys_manager.o: sys_manager.c