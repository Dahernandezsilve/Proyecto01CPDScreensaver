all:
	g++ -I src/include -L src/lib -o mainSecuencial mainSecuencial.cpp -lmingw32 -lSDL2main -lSDL2 -lSDL2_image
	g++ -I src/include -L src/lib -o mainParalelo mainParalelo.cpp -lmingw32 -lSDL2main -lSDL2 -lSDL2_image -lgomp

run:
	./mainParalelo 10