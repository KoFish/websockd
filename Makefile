#
# Makefile for websockd by Krister Svanlund <krister.svanlund@gmail.com>
#

# COMPILER
CC := gcc -m64 -std=c99
# COMPILER OPTIONS
CCOPT := -pedantic -Wall -Wshadow -Wpointer-arith -Wcast-qual -Wstrict-prototypes -Wmissing-prototypes

# SECONDARY OPTIONS AND LIBRARIES
COPT := -fdiagnostics-color=auto
LDD := -lev -D_POSIX_C_SOURCE=1

# NECESSARY FILES
EXEC := websockd
MAINC := src/daemon.c
EXTRAH := src/utils/utils.h src/config.h
OBJECTS := worker.o client.o worker_fork.o
UTIL_OBJECTS := socket_helpers.o ancmsg.o

# NECESSARY OBJECT FILES
_OBJ := $(addprefix obj/,$(OBJECTS)) $(addprefix obj/u/,$(UTIL_OBJECTS))


obj/u/%.o: src/utils/%.c src/utils/%.h
	${CC} ${CCOPT} -c $< -o $@ ${COPT}

obj/%.o: src/%.c src/%.h ${EXTRAH}
	${CC} ${CCOPT} -c $< -o $@ ${COPT} ${LDD}

${EXEC}: ${MAINC} ${_OBJ}
	${CC} ${CCOPT} $< -o $@ ${COPT} ${_OBJ} ${LDD}

clean:
	rm -v ${EXEC} ${_OBJ}
