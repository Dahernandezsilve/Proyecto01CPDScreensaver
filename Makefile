all:
#Windows
#g++ -I src/include -L src/lib -o mainSecuencial mainSecuencial.cpp -lmingw32 -lSDL2main -lSDL2 -lSDL2_image
#g++ -I src/include -L src/lib -o mainParalelo mainParalelo.cpp -lmingw32 -lSDL2main -lSDL2 -lSDL2_image -lgomp
	g++ -o mainSecuencial mainSecuencial.cpp `sdl2-config --cflags --libs` -lSDL2 -lSDL2_image
	g++ -o mainParalelo2 mainParalelo2.cpp  `sdl2-config --cflags --libs` -lSDL2 -lSDL2_image -fopenmp

run:
	./mainSecuencial 5 100 300 100
	./mainParalelo 5 100 300 100