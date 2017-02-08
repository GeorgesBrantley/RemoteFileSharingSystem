makesocks: port.c
	g++ -pthread -o rfss port.c
clean:
	rm -f rfss
