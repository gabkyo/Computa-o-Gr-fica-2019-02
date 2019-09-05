all:
	g++ -o trabalhocg trabalho.cpp -lfreeglut  -lGL -LGL 
	./trabalhocg Test_1/ #APAGAR


clean:
	rm -rf a.out
