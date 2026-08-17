processflow: processflow.c
	clang -Wall -Wextra -o processflow processflow.c
clean:
	rm -f processflow
	