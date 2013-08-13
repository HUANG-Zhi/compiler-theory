all: lexzer
lexzer: scanner.o
	g++ ./out/scanner.o -o ./out/lexzer
scanner.o: ./src/scanner.cpp 
	g++ -c ./src/scanner.cpp -o ./out/scanner.o
clean:
	rm -rf ./out/*.o
	rm -rf ./out/lexzer