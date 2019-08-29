#include <GL/glut.h>
#include <GL/gl.h>
#include <GL/glut.h>
#include <stdio.h>
#include <iostream>
#include <string>
#include "tinyxml2.h"
#include "tinyxml2.cpp"

using namespace std;
using namespace tinyxml2;

int main(int argc, char const *argv[])
{
    cout << argv[1]<<endl;
    XMLDocument doc;
    doc.LoadFile(argv[1]);
    cout<<doc.ErrorStr();
    doc.Print();
    XMLNode *pRoot =doc.FirstChild();
    if(pRoot==NULL){
        cout<<"XML Vazio ou Raiz de XML não encontrado."<<endl;
    }else {
        cout<< pRoot->FirstChildElement("circulo")->Attribute("raio") <<endl;
    }
    
    return 0;
}
