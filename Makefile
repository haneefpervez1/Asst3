all: first second
first: WTFclient.c
	-gcc -g -lm -Wall -Werror -fsanitize=address WTFclient.c -o WTF
second: WTFserver.c
	-gcc -g -lm -Wall -Werror -fsanitize=address WTFserver.c -o WTFserver
clean:
	rm -rf WTF WTFserver
