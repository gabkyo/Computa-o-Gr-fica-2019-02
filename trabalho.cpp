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
    string color;
    int cx,cy ,id,raio;

    bool operator <(const Circulo& c) const{
        return(id < c.id);
    }
}circulo_temp ;

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
    char *c;
    temp=root.FirstChildElement("svg").FirstChildElement("line").ToElement();
    if(temp==NULL || temp->QueryIntAttribute("id",&linha.id)!=XML_SUCCESS
     || temp->QueryIntAttribute("x1",&linha.x1)!=XML_SUCCESS
      || temp->QueryIntAttribute("x2",&linha.x2)!=XML_SUCCESS
       || temp->QueryIntAttribute("y1",&linha.y1)!=XML_SUCCESS
        || temp->QueryIntAttribute("y2",&linha.y2)!=XML_SUCCESS){
            ErroEstrutura();
            return false;
        }
    for ( temp=root.FirstChildElement("svg").FirstChildElement("circle").ToElement() ; temp!=NULL; temp=temp->NextSiblingElement("circle")){
        if (temp->QueryStringAttribute("fill",&c)==XML_SUCCESS){
            circulo_temp.color=string(c);
        }else ErroEstrutura();
        if(temp->QueryIntAttribute("cx",&circulo_temp.cx)!=XML_SUCCESS
            || temp->QueryIntAttribute("cy",&circulo_temp.cy)!=XML_SUCCESS
                || temp->QueryIntAttribute("raio",&circulo_temp.raio)!=XML_SUCCESS
                    || temp->QueryIntAttribute("id",&circulo_temp.id)!=XML_SUCCESS){
            ErroEstrutura();
            return false;
        }
        entidades.push_back(circulo_temp);        
    }
    if(entidades.size()<=0){
        cout<<"Nenhum circulo encontrado no arquivo da arena."<<endl;
        return false;
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

void drawCircle(Circulo circ)
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
    sort(entidades.begin(),entidades.end());
    glutInit(&argc, argv);
    glutInitWindowPosition(100, 100);
    glutInitWindowSize(50, 50);
    int raio=300;
    for(auto i:entidades){
        if (i.color=="blue"){
            raio=i.raio;
        }
        
    }
    glutInitWindowSize(raio, raio);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
    glClearColor(0, 0, 0, 1.0);
    glMatrixMode(GL_PROJECTION);
    glOrtho(0.0, raio, 0, raio, 0, 1);
    /*
    glutCreateWindow(janela.titulo.c_str());
    
    glLoadIdentity();
    
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