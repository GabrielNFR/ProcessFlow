processflow: processflow.c
	clang -Wall -Wextra -D_POSIX_C_SOURCE=200809L -o processflow processflow.c
clean:
	rm -f processflow
	