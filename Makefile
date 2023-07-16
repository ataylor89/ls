clean:
	if [ -d bin ]; then rm bin/*; rmdir bin; fi

install:
	if [ ! -d bin ]; then mkdir bin; fi;
	gcc src/main/*.c -o bin/ls

test: install
	gcc src/test/test.c -o bin/test