all:
	g++ -g -std=c++11 -o trabalhocg trabalho.cpp -lGL -lGLU -lglut
debug:
	gdb --args trabalhocg /Test_1/
run:
	./trabalhocg /Test_1/
clean:
	rm -rf trabalhocg
