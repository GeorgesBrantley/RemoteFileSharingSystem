makesocks: rfss.c
	g++ -pthread -o rfss rfss.c
clean:
	rm -f rfss
