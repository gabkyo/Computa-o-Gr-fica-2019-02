#include"GL/gl.h"
#include "GL/glut.h"
#include <stdio.h>
#include <iostream>
#include <string>
#include "tinyxml2.h"
#include "tinyxml2.cpp"

using namespace std;
using namespace tinyxml2;

struct Circulo{
    double raio,rgb[3],sobreposicao[3];
}circulo;

struct Janela{
    unsigned int dimensao[2];
    double fundo_rgb[3];
    string titulo;
}janela;

XMLDocument doc;
XMLElement* temp; //nodo  para manipular


void ErroEstrutura(){
    cout<<"XML Vazio ou Estrutura de XML errada."<<endl;
    return;
}

bool teste(char *arquivo){ // XML_SUCESS=0=false 
    if (doc.LoadFile(arquivo)) {
        cout<<doc.ErrorStr()<<endl;
        ErroEstrutura();
        return false;
    }
    XMLHandle root(&doc); //pointer para navegar
   temp=root.FirstChildElement("aplicacao").ToElement();

    
}

void display(){
    glClear(GL_COLOR_BUFFER_BIT); //limpa tudo

}


int main(int argc, char *argv[]){
    teste(argv[1]);
    cout << argv[1]<<endl;
    if(doc.LoadFile(argv[1])!= XML_SUCCESS){
        cout<<doc.ErrorStr();
        return 1;
    }
    

    
    glutInit(&argc,argv);
    glutInitDisplayMode(GLUT_RGB);
    glutInitWindowPosition(100,100);
    glutInitWindowSize( xy[0],xy[1]); 

    return 0;
}
