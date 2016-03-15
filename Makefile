LIBS := -lpthread -lrt

all: server client

server: server.c
	gcc -g spooler.c $^ -o $@ $(LIBS)

client: client.c
	gcc -g spooler.c $^ -o $@ $(LIBS)

clean:
	-rm server client
