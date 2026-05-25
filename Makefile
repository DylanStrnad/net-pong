# to compile: 
#     make pongS
#     make pongC
# to run on 1 computer:
#     ./pongS 2001
#     ./pongC 127.0.0.1 2001

pongS: pongServer.o paddle.o
	gcc -o pongS pongServer.o paddle.o -lncurses

pongC: pongClient.o paddle.o
	gcc -o pongC pongClient.o paddle.o -lncurses

clean:
	rm -f *.o pongServer pongClient