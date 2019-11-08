#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include "tinyxml2.h"
#include "tinyxml2.cpp"
#include <GL/glut.h>
#include <GL/freeglut.h>
#include <vector>
#include <cmath>
#include <unistd.h>

using namespace std;
using namespace tinyxml2;

////////////STRUCTS////////////

struct Linha{
    string color;
    double x1,x2,y1,y2;
}linha;

struct Circulo
{
    string color;
    double cx,cy,raio;

}circulo_temp,arena ;

struct Retangulo{
    double lado,altura;
    double cx,cy;
    string color;
}retan_temp;

struct Municao{
    Circulo tiro;
    double angulo;
};

struct Bomba{
    Circulo shell;
    int start;
    double angulo;
};

struct Nave{
    Circulo hitbox;
    int helice=0;
    int estado=0; // parado,pista,aumenta,voando,parado
    double angulo;
    double angulo_canhao;
    double angulo_o;
    double raio_o;
}jogador;

////////////trigonometric////////////

double distancia(double x1, double y1, double x2, double y2)
{
   return hypot(x2-x1,y2-y1);
}

double dist_linha(){
    return distancia(linha.x1,linha.y1,linha.x2,linha.y2);
}

double radianos(double x1, double x2, double y1, double y2){ //referencia 
    return atan2(x2-x1,y2-y1);
}


double rad_linha(){
    return radianos(linha.x1,linha.x2,linha.y1,linha.y2);
}

int rad2graus(double rad){
    return rad*180/M_PI;
}

double graus2rad(double graus){
    return graus*M_PI/180;
}

double ms2s(int ms){
    return double(ms)/1000.0;
}

double rad2pi(double a){ //de 0 a 2pi
    while(a>2*M_PI){
        a-=2*M_PI;
    }
    while(a<0){
        a+=2*M_PI;
    }
    return a;
}

////////////global vars////////////
vector<Circulo> terrestres;
vector<Circulo> voadores;
vector<Municao> tiros;
vector<Municao> inimigos;
vector<Bomba> bombas;
XMLDocument doc;
XMLElement *temp = NULL; //nodo  para manipular
int segmentos=50;

const int tock=50;
int start,delay[2],launch;
bool ataque[2]={true,true};


////////////SMALL functions////////////
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
    }else if(color=="yellow"){
        glColor3d(1.0,1.0,0.0);
    }else glColor3d(0.0,0.0,0.0);
}
////////////draw functions////////////
void init(){
    glClearColor(1, 1, 1, 1);
    glClear(GL_COLOR_BUFFER_BIT);
    glMatrixMode(GL_PROJECTION);
    glMatrixMode(GL_MODELVIEW);
    gluOrtho2D(arena.cx-arena.raio, arena.cx+arena.raio,arena.cy-arena.raio, arena.cy+arena.raio); //-y
}

void drawCircle(Circulo circ){
    double step = 2 * M_PI / segmentos, ox, oy;
    cor(circ.color);
    glBegin(GL_POLYGON);{
        for (double i = 0; i <= 2 * M_PI; i += step){
            ox=double(circ.raio*cos(i))+circ.cx;
            oy=double(circ.raio*sin(i))+circ.cy;
            glVertex2d(ox, oy);
        }
    }
    glEnd();
}

void drawLine(Linha l){
    cor(l.color);
    glBegin(GL_LINES);{
        glVertex2d(l.x1,l.y1);
        glVertex2d(l.x2,l.y2);
    }
    glEnd();
}

void drawRetangle(Retangulo r){
    cor(r.color);
    glBegin(GL_POLYGON);{
        glVertex2d(r.cx-r.lado/2,r.cy-r.altura/2);
        glVertex2d(r.cx-r.lado/2,r.cy+r.altura/2);
        glVertex2d(r.cx+r.lado/2,r.cy+r.altura/2);
        glVertex2d(r.cx+r.lado/2,r.cy-r.altura/2);
    }
    glEnd();
}

void drawNave(Nave nave){
    double theta,angx,angy;
    glPushMatrix();
    //transformado
    glTranslated(nave.hitbox.cx,nave.hitbox.cy,0);
    glRotated(-rad2graus(nave.angulo),0,0,1);
    glScaled(0.33,1,1);
    glTranslated(-nave.hitbox.cx,-nave.hitbox.cy,0);
    retan_temp.altura=nave.hitbox.raio/3;
    retan_temp.lado=nave.hitbox.raio*2/5;  
    drawCircle(nave.hitbox);
    circulo_temp.color="black";
    circulo_temp.raio=nave.hitbox.raio*0.4;
    circulo_temp.cx=nave.hitbox.cx;
    circulo_temp.cy=nave.hitbox.cy+circulo_temp.raio;
    drawCircle(circulo_temp);
    glTranslated(nave.hitbox.cx,nave.hitbox.cy,0);
    
    glBegin(GL_POLYGON);{//asa esquerda
        glVertex2d(-nave.hitbox.raio,-nave.hitbox.raio/3);
        glVertex2d(-nave.hitbox.raio,+nave.hitbox.raio/3);
        glVertex2d(-3*nave.hitbox.raio,0);
        glVertex2d(-3*nave.hitbox.raio,-2*nave.hitbox.raio/3);
    }
    glEnd();
    glBegin(GL_POLYGON);{//asa direita
        glVertex2d(+nave.hitbox.raio,-nave.hitbox.raio/3);
        glVertex2d(+nave.hitbox.raio,+nave.hitbox.raio/3);
        glVertex2d(+3*nave.hitbox.raio,0);
        glVertex2d(+3*nave.hitbox.raio,-2*nave.hitbox.raio/3);
    }
    glEnd();
    glBegin(GL_POLYGON);{//cauda
        glVertex2d(+nave.hitbox.raio/5,-nave.hitbox.raio);
        glVertex2d(-nave.hitbox.raio/5,-nave.hitbox.raio);
        glVertex2d(-nave.hitbox.raio/5,-nave.hitbox.raio/6);
        glVertex2d(+nave.hitbox.raio/5,-nave.hitbox.raio/6);
    }
    glEnd();
    //helices
    cor("yellow");
    angx=nave.hitbox.raio*0.5*sqrt(2)/2;
    angy=angx;
    glTranslated(2*nave.hitbox.raio,0,0);
    glRotated(nave.helice,0,0,1);
    glBegin(GL_POLYGON);{
        glVertex2d(0,0);
        glVertex2d(angx,angy);
        glVertex2d(angx,-angy);
        glVertex2d(-angx,angy);
        glVertex2d(-angx,-angy);
    }
    glEnd();
    glRotated(-nave.helice,0,0,1);
    glTranslated(-4*nave.hitbox.raio,0,0);
    glRotated(nave.helice,0,0,1);
    glBegin(GL_POLYGON);{
        glVertex2d(0,0);
        glVertex2d(angx,angy);
        glVertex2d(angx,-angy);
        glVertex2d(-angx,angy);
        glVertex2d(-angx,-angy);
    }
    glEnd();
    glRotated(-nave.helice,0,0,1);
    glTranslated(2*nave.hitbox.raio,0,0);
    //canhao consertar
    cor("red");
    glTranslated(0,nave.hitbox.raio,0);
    glRotated(-rad2graus(nave.angulo_canhao),0,0,1);
    glBegin(GL_POLYGON);{
        glVertex2d(+nave.hitbox.raio/5,0);
        glVertex2d(-nave.hitbox.raio/5,0);
        glVertex2d(-nave.hitbox.raio/5,nave.hitbox.raio/2);
        glVertex2d(+nave.hitbox.raio/5,nave.hitbox.raio/2);
    }
    glEnd();
    glRotated(rad2graus(nave.angulo_canhao),0,0,1);
    glTranslated(0,-nave.hitbox.raio,0);
    //desfazer transf
    glScaled(3,1,1);
    glRotated(rad2graus(nave.angulo),0,0,1);
    glTranslated(-nave.hitbox.cx,-nave.hitbox.cy,0);
    glPopMatrix();
}
////////////read functions////////////