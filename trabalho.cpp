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
}circulo ,modelo;

struct Janela{
    int dimensao[2];
    double fundo_rgb[3];
    string titulo;
}janela;

XMLDocument doc;
XMLElement* temp=NULL; //nodo  para manipular


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
   temp=root.FirstChildElement("aplicacao").FirstChildElement("circulo").ToElement();
   if(temp==NULL || temp->QueryDoubleAttribute("raio",&circulo.raio)!= XML_SUCCESS 
   || temp->QueryDoubleAttribute("corR",&circulo.rgb[0])!= XML_SUCCESS 
   || temp->QueryDoubleAttribute("corG",&circulo.rgb[1])!= XML_SUCCESS 
   || temp->QueryDoubleAttribute("corB",&circulo.rgb[2])!= XML_SUCCESS ){
        ErroEstrutura();
        cout<<"circulo"<<endl;
        return false;
   }
   temp=root.FirstChildElement("aplicacao").FirstChildElement("circuloModelo").ToElement();
   if(temp==NULL
   || temp->QueryDoubleAttribute("corR",&modelo.rgb[0])!= XML_SUCCESS 
   || temp->QueryDoubleAttribute("corG",&modelo.rgb[1])!= XML_SUCCESS 
   || temp->QueryDoubleAttribute("corB",&modelo.rgb[2])!= XML_SUCCESS
   || temp->QueryDoubleAttribute("corSobreposicaoR",&modelo.sobreposicao[0])!= XML_SUCCESS 
   || temp->QueryDoubleAttribute("corSobreposicaoG",&modelo.sobreposicao[1])!= XML_SUCCESS 
   || temp->QueryDoubleAttribute("corSobreposicaoB",&modelo.sobreposicao[2])!= XML_SUCCESS ){
        ErroEstrutura();
        cout<<"circuloModelo"<<endl;
        return false;
   }
   temp=root.FirstChildElement("aplicacao").FirstChildElement("janela").FirstChildElement("dimensao").ToElement();
   if(temp==NULL 
   || temp->QueryIntAttribute("largura",&janela.dimensao[0])!= XML_SUCCESS
   || temp->QueryIntAttribute("altura",&janela.dimensao[0])!= XML_SUCCESS){
       ErroEstrutura();
       cout<<"dimensao"<<endl;
       return false;
   }
   temp=root.FirstChildElement("aplicacao").FirstChildElement("janela").FirstChildElement("fundo").ToElement();
   if(temp==NULL 
   || temp->QueryDoubleAttribute("corR",&janela.fundo_rgb[0])!= XML_SUCCESS 
   || temp->QueryDoubleAttribute("corG",&janela.fundo_rgb[1])!= XML_SUCCESS 
   || temp->QueryDoubleAttribute("corB",&janela.fundo_rgb[2])!= XML_SUCCESS){
       ErroEstrutura();
       cout<<"fundo"<<endl;
       return false;
   }
   temp=root.FirstChildElement("aplicacao").FirstChildElement("janela").FirstChildElement("titulo").ToElement();
   if (temp==NULL || temp->GetText()==NULL){
       ErroEstrutura();
       cout<<"titulo"<<endl;
       return false; 
   }
   janela.titulo=string(temp->GetText());
    return true;
}

void display(){
    glClear(GL_COLOR_BUFFER_BIT); //limpa tudo

}


int main(int argc, char *argv[]){
    teste(argv[1]); //ver se o xml tem todas as informacoes necessarias
    
    glutInit(&argc,argv);
    glutInitDisplayMode(GLUT_RGB);
    glutInitWindowPosition(100,100);
    glutInitWindowSize( janela.dimensao[0],janela.dimensao[1]);
    glutInitDisplayString(janela.titulo.c_str()); 

    return 0;
}
