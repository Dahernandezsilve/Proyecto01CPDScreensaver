# 🖥️Proyecto01CPDScreensaver
### Proyecto de Paralelización con OpenMP

🏗️ Desarrollo de un programa que dibuja en pantalla un “screensaver” y que funciona con ejecución en forma paralela. 🤖

## 💻 Funcionalidades
- Makefile base para los integrantes.
- Código que genera una ventana en pantalla y que se utilizará para el desarrollo del programa secuencial y paralelo.
- Código que genera una versión de "Conway's The Game of Life" la cual se despliega en la ventana de los programas secuencial y paralelo.

## 🏛️ Información
OpenMP es una herramienta que facilita la transformación de un programa secuencial en paralelo, permitiendo la programación paralela de alto nivel. Es ideal para realizar cambios graduales en un programa secuencial para aprovechar múltiples recursos mediante ejecución paralela.

SDL (Simple DirectMedia Layer) es una biblioteca multiplataforma que proporciona un API para acceder a gráficos, sonido y otros elementos multimedia útiles para el desarrollo de elementos gráficos. Es significativamente utilizada en el desarrollo de videojuegos, emuladores y software que necesita del manejo de gráficos en tiempo real.

SDL_Image es una extensión de SDL, la cual permite la carga y manipulación de imágenes en distintos formatos.

## 🎯 Objetivos y Competencias
- Implementar y diseñar un programa para la paralelización de procesos con memoria compartida usando OpenMP.
- Aplicar el método PCAM y los conceptos de patrones de descomposición y programación para modificar un programa secuencial y volverlo paralelo.
- Realizar mejoras y modificaciones iterativas al programa para obtener mejores versiones.

## 📋 Requisitos
- Código de autoría propia en C/C++ con comentarios explicativos. 🖋️
- Historial del control de versiones del programa iniciando 2 semanas antes de la entrega (privado hasta el día de entrega). 📂
- Uso de OpenMP. 🚀
- Versiones secuencial y paralela(s) del algoritmo. 🔄
- Cálculo del speedup de cada versión de la solución paralela. 📈

## 📚 Contenido
Diseñar un programa que dibuje un “screensaver” y que corra de forma paralela. Comenzar con la versión secuencial y luego acelerarla usando OpenMP. Ejemplo en C++ con OpenMP y SDL: [Ejemplo de Screensaver](https://www.libsdl.org/).

### 🛠️ Herramientas y Tecnologías
- Utilizar SDL para gráficos y SDL Image para manejar imágenes. 🎨
- El screensaver debe recibir al menos un parámetro (N) que indica la cantidad de elementos a renderizar.
- Debe desplegar varios colores, idealmente generados de forma pseudoaleatoria.
- El tamaño del canvas debe ser de al menos 640x480 (w,h).
- Debe tener movimiento y ser estéticamente interesante.
- Debe incorporar elementos de física o trigonometría en sus cálculos.
- Debe mostrar los FPS (frames per second) para asegurar un rendimiento adecuado. 🕹️

### ⏳ Instrucciones de Compilación y Ejecución
Para compilar y ejecutar el proyecto. Es necesario contar con un entorno en Ubuntu que posea las librerías de SDL (con SDL_Image) y de OpenMP. Para hacer esto, debes clonar tu repositorio en una carpeta en Ubuntu.
Luego, procede a instalar las librerías con los siguientes comandos:

- sudo apt update (por si es necesario)
- sudo apt install libsdl2-dev
- sudo apt install libsdl2-image-dev

Una vez ya instaladas las librerías, es necesario compilar el Makefile que se encuentra en el proyecto para compilar los programas tanto secuencial como los programas paralelos. Esto puede hacerse con el comando
"make".

Si no hubo problemas al compilar el Makefile, utiliza "make run" para ejecutar los programas. De lo contrario, puedes compilarlos uno por uno usando los siguientes comandos:

- g++ -o mainSecuencial mainSecuencial.cpp `sdl2-config --cflags --libs` -lSDL2 -lSDL2_image
- g++ -o mainParalelo mainParalelo.cpp  `sdl2-config --cflags --libs` -lSDL2 -lSDL2_image -fopenmp
- g++ -o mainParalelo2 mainParalelo2.cpp  `sdl2-config --cflags --libs` -lSDL2 -lSDL2_image -fopenmp
- g++ -o mainParalelo3 mainParalelo3.cpp  `sdl2-config --cflags --libs` -lSDL2 -lSDL2_image -fopenmp
- g++ -o mainParalelo4 mainParalelo4.cpp  `sdl2-config --cflags --libs` -lSDL2 -lSDL2_image -fopenmp -O2
- ./mainSecuencial <max_gifs> <num_glider> <num_guns> <num_smallGliders>
- ./mainParalelo <max_gifs> <num_glider> <num_guns> <num_smallGliders>

### 💡 Recomendaciones
- Medir el tiempo de ejecución para garantizar al menos 60 fps o el valor más cercano. ⏱️
- Utilizar otras técnicas de paralelización como el uso de procesos en lugar de hilos.
- Introducir otras métricas de rendimiento y aumentar la precisión de los resultados.
- Considerar si el entorno de ejecución beneficia o perjudica el desarrollo.
- Comparar rendimientos respecto a otras librerías gráficas como OpenGL o de paralelización como CUDA.


## 👨‍🏫 Docente
Sebastián Galindo

## 🏫 Universidad del Valle de Guatemala
Computación Paralela y Distribuida

Semestre 2, 2024

# 😁 Autores
- Diego Alexander Hernández Silvestre | Carné 21270 🛻
- Linda Inés Jiménez Vides | Carné 21169 🏎️
- Mario Antonio Guerra Morales | Carné 21008 🚗
