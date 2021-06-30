CC = g++
CFLAGS = -g -Wall

server: main.o response.o socket.o                                       
	${CC} main.o response.o socket.o ${CFLAGS} -o $@
%.o: %.c                      
	${CC} $< ${CFLAGS}-lpthread -c          
.PHONY: clean
clean:                             
	@rm -rf *.o
