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
int segmentos=12;

struct Linha{
    int x1,x2,y1,y2,id;
}linha;

struct Circulo
{
    double raio;
    string color;
    int cx,cy ,id;
} ;

vector<Circulo> entidades;

XMLDocument doc;
XMLElement *temp = NULL; //nodo  para manipular

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

bool teste(char *arquivo)
{ // XML_SUCESS=0=false
    FILE *i = fopen(arquivo, "r");
    string arena;
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
    fclose(i);
    XMLHandle root(&doc); //pointer para navegar
    temp = root.FirstChildElement("aplicacao").FirstChildElement("arquivoDaArena").FirstChildElement("caminho").ToElement();
    Circulo circulo;
    if(temp==NULL){
        ErroEstrutura();
        return false;
    }
    arena=temp->GetText();
    if(arena[0]=='/'){
        arena='.'+arena;
    }
    temp=root.FirstChildElement("aplicacao").FirstChildElement("arquivoDaArena").FirstChildElement("nome").ToElement();
     if(temp==NULL){
        ErroEstrutura();
        return false;
    }
    arena+=temp->GetText()+'.';
    temp=root.FirstChildElement("aplicacao").FirstChildElement("arquivoDaArena").FirstChildElement("tipo").ToElement();
     if(temp==NULL){
        ErroEstrutura();
        return false;
    }
    arena+=temp->GetText();
    i=fopen(arena.c_str(),"r");
    if (i == NULL)
    {
        cout << "FILE ARENA";
        ErroIO();
        return false;
    }
    if (doc.LoadFile(i) != XML_SUCCESS)
    {
        cout << "doc ARENA";
        ErroEstrutura();
        cout << doc.ErrorStr() << endl;
        return false;
    }
    fclose(i);
    root=XMLHandle(&doc);
    ;
    for ( temp=root.FirstChildElement("svg").FirstChildElement("circle").ToElement() ; temp!=NULL; temp=temp->NextSiblingElement("circle")){
        if (temp->QueryStringAttribute())
        {
            /* code */
        }
        
    }
    
    return true;
}

double distancia(int x1, int x2, int y1, int y2)
{
    double a = double(pow((max(x1, x2) - min(x1, x2)), 2)),
           b = double(pow((max(y1, y2) - min(y1, y2)), 2));
    return sqrt(a + b);
}


void idle()
{
    glutPostRedisplay();
}

void click(int button, int state, int x, int y)
{
    
}

void passivemove(int x, int y)
{
   
}

void drag(int x, int y)
{
    
}

void drawCircle(int x, int y)
{
    double step = 2 * pi / segmentos, ox, oy;
    glBegin(GL_LINE_LOOP);
    {
        for (double i = 0; i <= 2 * pi; i += step)
        {
            //ox = 2 * (double(x) + circulo.raio * cos(i)) / double(janela.dimensao[0]) - 1.0;
            //oy = 2 * (double(y) + circulo.raio * sin(i)) / double(janela.dimensao[1]) + 1.0;
            //glVertex2f(ox, oy);
        }
    }
    glEnd();
}

void display()
{
    glClear(GL_COLOR_BUFFER_BIT); //limpa tudo
    
    glFlush();
}

int main(int argc, char *argv[])
{
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
    /*glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
    glutInitWindowPosition(100, 100);
    glutInitWindowSize(janela.dimensao[0], janela.dimensao[1]);
    glutCreateWindow(janela.titulo.c_str());
    glClearColor(janela.fundo_rgb[0], janela.fundo_rgb[1], janela.fundo_rgb[2], 1.0);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0, janela.dimensao[0], 0, janela.dimensao[1], 0, 0);
    //fim do setup
    glutDisplayFunc(display);
    glutMouseFunc(click);
    glutMotionFunc(drag);
    glutPassiveMotionFunc(passivemove);
    ;;fim das definicoes de estados
    glutMainLoop();
    */
    return 0;
}