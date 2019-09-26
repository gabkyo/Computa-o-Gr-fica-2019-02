#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <cstring>
#include <vector>
#include <algorithm>
#include <cmath>
#include "tinyxml2.h"
#include "tinyxml2.cpp"
#include <GL/glut.h>
#include <GL/freeglut.h>
#include <limits.h>
#include <iterator>

using namespace std;
using namespace tinyxml2;

double pi=3.14;
int segmentos=30;

struct Linha{
    string color;
    int x1,x2,y1,y2;
}linha;

struct Circulo
{
    string color;
    int cx,cy,raio;

}circulo_temp, jogador,arena ;

vector<Circulo> terrestres;
vector<Circulo> voadores;

XMLDocument doc;
XMLElement *temp = NULL; //nodo  para manipular

void init(){
    glClearColor(1, 1, 1, 1);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(arena.cx-arena.raio, arena.cx+arena.raio,arena.cy+arena.raio, arena.cy-arena.raio, -1, 1);
}

void ErroEstrutura()
{
    cout << "XML Vazio ou Estrutura de XML errada." << endl;
    return;
}

void ErroIO()
{
    cout << "Arquivo nao encontrado" << endl;
    return;
}

void cor(string color){
    if(color=="blue"){
        glColor3d(0.0,0.0,1.0);
    }else if(color=="orange"){
        glColor3d(1.0,0.5,0.0);
    }else if(color=="red"){
        glColor3d(1.0,0.0,0.0);
    }else if(color=="green"){
        glColor3d(0.0,1.0,0.0);
    }else glColor3d(0.0,0.0,0.0);
}

bool teste(char *arquivo)
{ // XML_SUCESS=0=false
    FILE *i = fopen(arquivo, "r");
    string svg;
    const char *c;
    if (i == NULL)
    {
        cout << "FILE";
        ErroIO();
        return false;
    }
    if (doc.LoadFile(i) != XML_SUCCESS)
    {
        cout << "doc";
        ErroEstrutura();
        cout << doc.ErrorStr() << endl;
        return false;
    }
    XMLHandle root(&doc); //pointer para navegar
    temp = root.FirstChildElement("aplicacao").FirstChildElement("arquivoDaArena").FirstChildElement("caminho").ToElement();
    if(temp==NULL){
        ErroEstrutura();
        return false;
    }
    svg=temp->GetText();
    if(svg[0]=='/'){
        svg='.'+svg;
    }else if(svg[0]=='~'){
        c=getenv("HOME");
        svg=svg.substr(1);
        svg=string(c)+svg;
    }
    temp=root.FirstChildElement("aplicacao").FirstChildElement("arquivoDaArena").FirstChildElement("nome").ToElement();
     if(temp==NULL){
        ErroEstrutura();
        return false;
    }
    svg=svg+temp->GetText()+'.';
    temp=root.FirstChildElement("aplicacao").FirstChildElement("arquivoDaArena").FirstChildElement("tipo").ToElement();
     if(temp==NULL){
        ErroEstrutura();
        return false;
    }
    svg=svg+temp->GetText();
    doc.Clear();
    c=svg.c_str();
    freopen(c,"r",i);
    if (doc.LoadFile(svg.c_str()) != XML_SUCCESS)
    {
        cout << "doc ARENA"<<endl;
        ErroEstrutura();
        cout << doc.ErrorStr() << endl;
        return false;
    }
    root=XMLHandle(&doc);
    temp=root.FirstChildElement("svg").FirstChildElement("line").ToElement();
    if(temp==NULL || temp->QueryIntAttribute("x1",&linha.x1)!=XML_SUCCESS
      || temp->QueryIntAttribute("x2",&linha.x2)!=XML_SUCCESS
       || temp->QueryIntAttribute("y1",&linha.y1)!=XML_SUCCESS
        || temp->QueryIntAttribute("y2",&linha.y2)!=XML_SUCCESS){
            cout<<"linha"<<endl;
            ErroEstrutura();
            fclose(i);
            return false;
        }
    for ( temp=root.FirstChildElement("svg").FirstChildElement("circle").ToElement() ; temp!=NULL; temp=temp->NextSiblingElement("circle")){
        if (temp->QueryStringAttribute("fill",&c)==XML_SUCCESS){
            circulo_temp.color=string(c);
        }else {
            cout<<"circulo"<<endl;
            ErroEstrutura();
            return false;
        }
        if(temp->QueryIntAttribute("cx",&circulo_temp.cx)!=XML_SUCCESS
            || temp->QueryIntAttribute("cy",&circulo_temp.cy)!=XML_SUCCESS
                || temp->QueryIntAttribute("r",&circulo_temp.raio)!=XML_SUCCESS){
            
            cout<<"circulo "<<temp<<endl;
            ErroEstrutura();
            fclose(i);
            return false;
        }
        if(circulo_temp.color=="blue"){
            arena=circulo_temp;
        }else if(circulo_temp.color=="green"){
            jogador=circulo_temp;
        }else if(circulo_temp.color=="red"){
            voadores.push_back(circulo_temp);
        }else if(circulo_temp.color=="orange"){
            terrestres.push_back(circulo_temp);
        }else {
            cout<<"Cor indefinida: "<<circulo_temp.color<<endl;
            fclose(i);
            return false;
        }     
    } 
    if(arena.raio<=0 || jogador.raio<=0 || voadores.size()==0 || terrestres.size()==0){
        cout<<"Circulos invalidos ou faltando"<<endl;
        fclose(i);
        return false;
    }
    fclose(i);
    return true;
}

double distancia(int x1, int x2, int y1, int y2)
{
    double a = double(pow((max(x1, x2) - min(x1, x2)), 2)),
           b = double(pow((max(y1, y2) - min(y1, y2)), 2));
    return sqrt(a + b);
}

void teclado(unsigned char key,int x, int y){

}

void drawCircle(Circulo circ){
    double step = 2 * pi / segmentos, ox, oy;
    cor(circ.color);
    glBegin(GL_POLYGON);{
        for (double i = 0; i <= 2 * pi; i += step){
            ox=double(circ.raio*cos(i))+double(circ.cx);
            oy=double(circ.raio*sin(i))+double(circ.cy);
            glVertex2d(ox, oy);
        }
    }
    glEnd();
}

void drawLine(Linha l){
    cor(l.color);
    glBegin(GL_LINES);{
        glVertex2i(l.x1,l.y1);
        glVertex2i(l.x2,l.y2);
    }
    glEnd();
}

void display()
{  
    glClear(GL_COLOR_BUFFER_BIT); //limpa tudo
    drawCircle(arena);
    for(auto i:terrestres){
        drawCircle(i);
    }
    for(auto i:voadores){
        drawCircle(i);
    }
    drawLine(linha);
    drawCircle(jogador);
    glFlush();
}

int main(int argc, char *argv[])
{
    if(argc!=2){
        cout<<"Favor informar apenas o caminho do config."<<endl;
        return 1;
    }
    string temp = string(argv[1]);
    temp.append("config.xml");
    if(temp.at(0)=='/'){
        temp="."+temp;
    }
    char arquivo[temp.length() + 1];
    strcpy(arquivo, temp.c_str());
    if (!teste(arquivo))
    {
        return 1;
    }
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
    glutInitWindowSize(arena.raio*2, arena.raio*2);
    glutCreateWindow("trabalhocg");
    init();
    
    //fim do setup
    glutDisplayFunc(display);
    glutKeyboardFunc(teclado);
    //fim das definicoes de estados
    glutMainLoop();

    return 0;
}
