all:
	g++ trabalho.cpp -lGL -lGL -lglut
	./a.out Test_1/config.xml #APAGAR


clean:
	rm -rf a.out
