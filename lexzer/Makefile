all: scanner.o lexzer.o
	g++ ./out/scanner.o ./out/lexzer.o -o ./out/lexzer

scanner.o: ./src/scanner.cpp 
	g++ -c ./src/scanner.cpp -o ./out/scanner.o

lexzer.o: ./src/lexzer.cpp
	g++ -c ./src/lexzer.cpp -o ./out/lexzer.o
	
clean:
	rm -rf ./out/*