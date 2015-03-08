#   Copyright 2015 Krister Svanlund <krister.svanlund@gmail.com>
#
#   Licensed under the Apache License, Version 2.0 (the "License");
#   you may not use this file except in compliance with the License.
#   You may obtain a copy of the License at
#
#       http://www.apache.org/licenses/LICENSE-2.0
#
#   Unless required by applicable law or agreed to in writing, software
#   distributed under the License is distributed on an "AS IS" BASIS,
#   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#   See the License for the specific language governing permissions and
#   limitations under the License.
#

#
# Makefile for websockd
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
