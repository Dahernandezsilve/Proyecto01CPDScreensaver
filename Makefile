all:
	g++ -I src/include -L src/lib -o mainSecuencial mainSecuencial.cpp -lmingw32 -lSDL2main -lSDL2