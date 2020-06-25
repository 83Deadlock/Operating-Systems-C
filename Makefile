CC=gcc
DEPS = argus.h executar.h historico.h
OBJ = argusd.o executar.o historico.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $<

all: argusd argus
	@mkfifo -m 0666 pipe_cliente_servidor
	@mkfifo -m 0666 pipe_servidor_cliente
	@> log
	@> log.idx

argusd: $(OBJ)
	$(CC) -o $@ $^

argus: argus.o
	$(CC) -o $@ argus.o

.PHONY: clean

clean:
	rm -f *.o argusd argus log log.idx
	@unlink pipe_servidor_cliente
	@unlink pipe_cliente_servidor