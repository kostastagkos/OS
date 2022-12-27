os: os.o os_funcs.o
	gcc -Wall -Werror -std=c9x os.o os_funcs.o -o os -lm

os.o: os.c
	gcc -Wall -Werror -std=c9x -c os.c

os_funcs.o: os_funcs.c 
	gcc -Wall -Werror -std=c9x -c os_funcs.c

clean:
	rm -f os *.o os

