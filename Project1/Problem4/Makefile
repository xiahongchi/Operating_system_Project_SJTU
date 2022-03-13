all: client.c server.c server.h general.h
	gcc -o client client.c general.h -lpthread
	gcc -o server server.c server.h general.h -lpthread
clean:
	rm client server