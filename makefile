all: mandelbrot

clean:
	rm mandelBrot
	rm mandelCalc
	rm mandelDisp

mandelbrot: mandelBrot-slingutl.cpp mandelCalc-slingutl.cpp mandelDisp-slingutl.cpp
	g++ -std=c++11 -o mandelBrot mandelBrot-slingutl.cpp
	g++ -std=c++11 -o mandelCalc mandelCalc-slingutl.cpp
	g++ -std=c++11 -o mandelDisp mandelDisp-slingutl.cpp

run: mandelBrot
	./mandelBrot
