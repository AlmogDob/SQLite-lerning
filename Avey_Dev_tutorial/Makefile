CFLAGS = -Wall -Wextra -lm -lsqlite3
CCHECKS = -fsanitize=address

main: build_main run_main clean_main  
	@echo [INFO] done

build_main: main.c
	@echo [INFO] building
	@gcc ./main.c $(CFLAGS) -o main

run_main:
	@echo
	./main

clean_main:
	@echo
	rm main

debug_build_main: main.c
	@gcc ./main.c $(CFLAGS) -g -o main
# valgrind -s --leak-check=full ./main
# cloc --exclude-lang=JSON,make .
