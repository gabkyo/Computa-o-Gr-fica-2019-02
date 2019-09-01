all:
	g++ -o trabalhocg trabalho.cpp -lfreeglut  -lGL -LGL 
	./trabalhocg Test_1/config.xml #APAGAR


clean:
	rm -rf a.out
