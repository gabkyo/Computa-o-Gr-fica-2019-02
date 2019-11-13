exec=trabalhocg
main=trabalho.cpp
target=./
all:
	g++ -g -std=c++11 -o $(exec) $(main) -lGL -lGLU -lglut
clean:
	rm -rf $(exec)

