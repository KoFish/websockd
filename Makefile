CC := gcc -m64 -std=c99 -pedantic -Wall -Wshadow -Wpointer-arith -Wcast-qual -Wstrict-prototypes -Wmissing-prototypes
COPT := -fdiagnostics-color=auto
LDD := -lev -D_POSIX_C_SOURCE=1
OBJ := obj/worker.o obj/ancmsg.o obj/utils.o obj/client.o obj/worker_fork.o

obj/%.o: src/%.c src/%.h src/config.h
	${CC} -c $< -o $@ ${LDD} ${COPT}

websocketd: src/daemon.c ${OBJ}
	${CC} $< -o $@ ${LDD} ${COPT} ${OBJ}

clean:
	rm -v websocketd ${OBJ}
