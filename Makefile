all: first second
first: WTFclient.c
	-gcc -g -lm -Wall -Werror -fsanitize=address WTFclient.c -o WTF -lssl -lcrypto
second: WTFserver.c
	-gcc -g -lm -Wall -Werror -fsanitize=address WTFserver.c -o WTFserver -lssl -lcrypto
clean:
	rm -rf WTF WTFserver
