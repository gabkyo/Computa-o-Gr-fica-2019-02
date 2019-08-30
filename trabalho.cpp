#include <GL/glut.h>
#include <GL/gl.h>
#include <stdio.h>
#include <iostream>
#include <string>
#include "tinyxml2.h"
#include "tinyxml2.cpp"

using namespace std;
using namespace tinyxml2;

int ErroEstrutura(){
    cout<<"XML Vazio ou Estrutura de XML errada."<<endl;
    return 1;
}


int main(int argc, char const *argv[])
{
    XMLDocument doc;
    XMLHandle root(&doc);
    XMLElement* temp;
    int xy[2];
    double rgb[3];
    cout << argv[1]<<endl;
    if(doc.LoadFile(argv[1])!= XML_SUCCESS){
        cout<<doc.ErrorStr();
        return 1;
    }
    temp= root.FirstChildElement("aplicacao").FirstChildElement("janela").FirstChildElement("dimensao").ToElement();
    if (temp && temp->QueryAttribute("largura")==XML_SUCCESS && temp->QueryAttribute("altura")==XML_SUCCESS){
    glutInitWindowSize( temp->Attribute("largura"), temp->Attribute("altura") );
    }else return ErroEstrutura();

    return 0;
}
