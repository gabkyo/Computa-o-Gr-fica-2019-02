all:
	g++ -g -std=c++11 -o trabalhocg trabalho.cpp -lGL -lGLU -lglut
clean:
	rm -rf trabalhocg
debug:
	gdb --args trabalhocg ./
run:
	./trabalhocg ./
