#include <stdio.h>
#include <iostream>
#include <string>
#include <cstring>
#include <list>
#include <cmath>
#include "tinyxml2.h"
#include "tinyxml2.cpp"
#include <GL/glut.h>
#include <GL/freeglut.h>

using namespace std;
using namespace tinyxml2;

struct Circulo{
    double raio,rgb_circulo[3],rgb_modelo[3],sobreposicao[3];
}circulo;



struct Janela{
    int dimensao[2];
    double fundo_rgb[3];
    string titulo;
}janela;



XMLDocument doc;
XMLElement* temp=NULL; //nodo  para manipular
list<pair<int,int>> marcados;
int mouse[2];
const double pi=3.14;
const int segmentos=12;

void ErroEstrutura(){
    cout<<"XML Vazio ou Estrutura de XML errada."<<endl;
    return;
}

bool teste(char *arquivo){ // XML_SUCESS=0=false 
    if (doc.LoadFile(arquivo)) {
        cout<<"Arquivo nao encontrado"<<endl;
        cout<<doc.ErrorStr()<<endl;
        return false;
    }
    XMLHandle root(&doc); //pointer para navegar
   temp=root.FirstChildElement("aplicacao").FirstChildElement("circulo").ToElement();
   if(temp==NULL || temp->QueryDoubleAttribute("raio",&circulo.raio)!= XML_SUCCESS 
   || temp->QueryDoubleAttribute("corR",&circulo.rgb_circulo[0])!= XML_SUCCESS 
   || temp->QueryDoubleAttribute("corG",&circulo.rgb_circulo[1])!= XML_SUCCESS 
   || temp->QueryDoubleAttribute("corB",&circulo.rgb_circulo[2])!= XML_SUCCESS ){
        ErroEstrutura();
        cout<<"circulo"<<endl;
        return false;
   }
   temp=root.FirstChildElement("aplicacao").FirstChildElement("circuloModelo").ToElement();
   if(temp==NULL
   || temp->QueryDoubleAttribute("corR",&circulo.rgb_modelo[0])!= XML_SUCCESS 
   || temp->QueryDoubleAttribute("corG",&circulo.rgb_modelo[1])!= XML_SUCCESS 
   || temp->QueryDoubleAttribute("corB",&circulo.rgb_modelo[2])!= XML_SUCCESS
   || temp->QueryDoubleAttribute("corSobreposicaoR",&circulo.sobreposicao[0])!= XML_SUCCESS 
   || temp->QueryDoubleAttribute("corSobreposicaoG",&circulo.sobreposicao[1])!= XML_SUCCESS 
   || temp->QueryDoubleAttribute("corSobreposicaoB",&circulo.sobreposicao[2])!= XML_SUCCESS ){
        ErroEstrutura();
        cout<<"circuloModelo"<<endl;
        return false;
   }
   temp=root.FirstChildElement("aplicacao").FirstChildElement("janela").FirstChildElement("dimensao").ToElement();
   if(temp==NULL 
   || temp->QueryIntAttribute("largura",&janela.dimensao[0])!= XML_SUCCESS
   || temp->QueryIntAttribute("altura",&janela.dimensao[1])!= XML_SUCCESS){
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

double distancia(int x1,int x2, int y1, int y2){
    double a=double(pow((max(x1,x2)-min(x1,x2)),2)),
    b=double(pow((max(y1,y2)-min(y1,y2)),2));
    return sqrt(a+b);
}

bool overlap(int x, int y){
    for(auto const &i:marcados){
        if (distancia(x,i.first,y,i.second)<=2*circulo.raio){
            return true;
        }
    }
    return false;
}

void click(int button, int state,int x,int y){
    if (button==GLUT_LEFT_BUTTON && state==GLUT_UP){
        if (!overlap(x,y)){
            marcados.push_back(make_pair(x,y));
            glutPostRedisplay();
        }
        
    }
    
}

void passivemove(int x, int y){
    mouse[0]=x;
    mouse[1]=y;
}

void idle(){
    glutPostRedisplay();
}

void drawCircle(int x, int y){
    double step=2*pi/segmentos;
    glBegin(GL_POLYGON);{
    for (double i = 0; i <= 2*pi; i+=step){
        glVertex2f(x+circulo.raio*cos(i)/janela.dimensao[0],y+circulo.raio*sin(i)/janela.dimensao[1]);
        cout<<x+circulo.raio*cos(i)/janela.dimensao[0]<<" "<<y+circulo.raio*sin(i)/janela.dimensao[1]<<endl;
    }
    }glEnd();
}

void display(){
    glClear(GL_COLOR_BUFFER_BIT); //limpa tudo
    glColor3f(circulo.rgb_circulo[0],circulo.rgb_circulo[1],circulo.rgb_circulo[2]);
    for(auto const &i:marcados){
        drawCircle(i.first,i.second);
    }
    if(overlap(mouse[0],mouse[1])){
        glColor3f(circulo.sobreposicao[0],circulo.sobreposicao[1],circulo.sobreposicao[2]);
    }else glColor3f(circulo.rgb_modelo[0],circulo.rgb_modelo[1],circulo.rgb_modelo[2]);   
    drawCircle(mouse[0],mouse[1]);
    glFlush();
}


int main(int argc, char *argv[]){
    string temp=string(argv[1]);
    temp.append("config.xml");
    if(temp[0]=='\\'){
        temp.erase(0);
    }
    char *arquivo=new char[temp.length()+1];
    strcpy(arquivo,temp.c_str());
    cout<<arquivo<<endl;
    teste(arquivo);
    cout<<"circulo "<<circulo.raio<<" "<<circulo.rgb_circulo[0]<<" "<<circulo.rgb_circulo[1]<<" "<<circulo.rgb_circulo[2]<<" "<<circulo.rgb_modelo<<" "<<circulo.sobreposicao<<endl;
    cout<<"janela "<<janela.dimensao[0]<<" "<<janela.dimensao[1]<<" "<<janela.fundo_rgb<<" "<<janela.titulo<<endl;
    glutInit(&argc,argv);
    glutInitDisplayMode(GLUT_SINGLE|GLUT_RGB);
    glutInitWindowPosition(100,100);
    glutInitWindowSize( janela.dimensao[0],janela.dimensao[1]); 
    glutCreateWindow(janela.titulo.c_str());
    glClearColor(janela.fundo_rgb[0],janela.fundo_rgb[1],janela.fundo_rgb[2],1.0);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0,janela.dimensao[0],janela.dimensao[1],0,-1.0,-1.0);
    //fim do setup
    glutDisplayFunc(display);
    glutMouseFunc(click);
    glutMotionFunc(passivemove);
    glutPassiveMotionFunc(passivemove);
    //fim das definicoes de estados
    glutMainLoop();
    return 0;
}
