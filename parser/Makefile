all: stack.o scanner.o syn_sem.o parser.o
	g++ ./out/stack.o ./out/scanner.o ./out/syn_sem.o ./out/parser.o -o ./out/parser

stack.o: ./src/stack.cpp 
	g++ -c ./src/stack.cpp -o ./out/stack.o

scanner.o: ./src/scanner.cpp 
	g++ -c ./src/scanner.cpp -o ./out/scanner.o

syn_sem.o: ./src/syn_sem.cpp
	g++ -c ./src/syn_sem.cpp -o ./out/syn_sem.o

parser.o: ./src/parser.cpp
	g++ -c ./src/parser.cpp -o ./out/parser.o
	
clean:
	rm -rf ./out/*