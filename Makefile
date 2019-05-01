all: first second
first: WTF.c
	-gcc -g -lm -Wall -Werror -fsanitize=address WTF.c -o WTF
second: WTFserver.c
	-gcc -g -lm -Wall -Werror -fsanitize=address WTFserver.c -o WTFserver
clean:
	rm -rf WTF WTFserver
