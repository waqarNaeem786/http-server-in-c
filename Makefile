TARGET = server

server : server.c
	gcc -Wall server.c -o server
clean:
	$(RM) $(TARGET)
