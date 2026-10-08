# make        build bin/udp_server and bin/udp_client
# make test   run the end-to-end test (tests/run_test.sh)
# make clean  remove binaries

CC     = gcc
CFLAGS = -std=c11 -Wall -Wextra -pedantic -O2

all: bin/udp_server bin/udp_client

bin/%: src/%.c | bin
	$(CC) $(CFLAGS) $< -o $@

bin:
	mkdir -p bin

test: all
		bash tests/run_test.sh

clean:
	rm -rf bin

.PHONY: all test clean
