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
    double x0,y0;//posicao inicial
    int helice=0;
    int estado=0; // parado,pista,aumenta,voando,parado
    double angulo;//radianos
    double angulo_canhao;//radianos
    double angulo_o;//angulo original
    double raio_o;
    double vel;//multiplicador
    double veltiro;//multiplicador
    double a;//aceleracao real
    double v;//velocidade real
    bool enabled;
}jogador, nave_temp;

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
vector<Nave> bases;
vector<Nave> inimigos;
vector<Municao> tiros;
vector<Municao> tiros_inimigos;
vector<Bomba> bombas;
XMLDocument doc;
XMLElement *temp = NULL; //nodo  para manipular
int segmentos=50;
double multiplicador=1,freqTiro;
const int tock=50;
int start,delay[2],launch,score_total,score_atual=0;
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

double distancia(Circulo a,Circulo b){
    return distancia(a.cx,a.cy,b.cx,b.cy);
}

bool inbound(){
    return distancia(jogador.hitbox,arena)<=double(arena.raio-jogador.hitbox.raio);
}

bool contato(Circulo a, Circulo b){
    return distancia(a.cx,a.cy,b.cx,b.cy)<=double(a.raio+b.raio);
}
////////////draw functions////////////
void init(){
    glClearColor(1, 1, 1, 1);
    glClear(GL_COLOR_BUFFER_BIT);
    glMatrixMode(GL_PROJECTION);
    glMatrixMode(GL_MODELVIEW);
    gluOrtho2D(arena.cx-arena.raio, arena.cx+arena.raio,arena.cy-arena.raio, arena.cy+arena.raio); //-y
}

void reset_nave(Nave *nave){
    nave->angulo=nave->angulo_o;
    nave->hitbox.cx=nave->x0;
    nave->hitbox.cy=nave->y0;
    nave->v=0;
    nave->angulo_canhao=0;
    nave->estado=0;
    nave->a=0;
    nave->helice=0;
    nave->enabled=true;
    nave->hitbox.raio=nave->raio_o;
}

void reset(){
    start=0;
    launch=0;
    multiplicador=1;
    for (auto i: inimigos){
        reset_nave(&i);
    }
    for (auto i: bases){
        reset_nave(&i);
    }
    reset_nave(&jogador);
    tiros.clear();
    inimigos.clear();
    bombas.clear();
    glutPostRedisplay();
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

void addTiroInimigo(Nave *a){
    Municao temp;
    temp.tiro.color="red";
    temp.tiro.raio=a->hitbox.raio/5;
    temp.angulo=a->angulo_canhao+a->angulo;
    temp.tiro.cx=a->hitbox.cx+a->hitbox.raio*sin(a->angulo)+a->hitbox.raio*0.5*sin(a->angulo_canhao);
    temp.tiro.cy=a->hitbox.cy+a->hitbox.raio*cos(a->angulo)+a->hitbox.raio*0.5*cos(a->angulo_canhao);
    tiros.push_back(temp);
}
void addTiro(){
    Municao temp;
    temp.tiro.color="yellow";
    temp.tiro.raio=jogador.hitbox.raio/5;
    temp.angulo=jogador.angulo_canhao+jogador.angulo;
    temp.tiro.cx=jogador.hitbox.cx+jogador.hitbox.raio*sin(jogador.angulo)+jogador.hitbox.raio*0.5*sin(jogador.angulo_canhao);
    temp.tiro.cy=jogador.hitbox.cy+jogador.hitbox.raio*cos(jogador.angulo)+jogador.hitbox.raio*0.5*cos(jogador.angulo_canhao);
    tiros.push_back(temp);
}
void addBomba(){

}


////////////read functions////////////
bool teste(string arquivo)
{ // XML_SUCESS=0=false
    double i_vel,i_veltiro;
    string svg;
    svg=arquivo;
    if(arquivo[0]=='/' && svg.find("home")==string::npos){
        svg="."+arquivo;
    }
    else if(arquivo[0]=='~'){
        svg=string(getenv("HOME"))+arquivo.substr(1);
        svg=svg.substr(1);
    } 
    FILE *i = fopen(svg.c_str(), "r");
    const char *c;
    svg=arquivo.substr(1);
    if (i == NULL)
    {
        i=fopen(svg.c_str(),"r");
        if(i==NULL){
            cout << "FILE ";
            ErroIO();
            return false;
        }
        
    }
    if (doc.LoadFile(i) != XML_SUCCESS)
    {
        cout << "doc ";
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
    temp=root.FirstChildElement("aplicacao").FirstChildElement("jogador").ToElement();
    if(temp==NULL || temp->QueryDoubleAttribute("vel",&jogador.vel)!=XML_SUCCESS){
        ErroEstrutura();
        return false;
    }
    if(temp==NULL || temp->QueryDoubleAttribute("velTiro",&jogador.veltiro)!=XML_SUCCESS){
        ErroEstrutura();
        return false;
    }
    temp=root.FirstChildElement("aplicacao").FirstChildElement("inimigo").ToElement();
    if(temp==NULL || temp->QueryDoubleAttribute("vel",&i_vel)!=XML_SUCCESS){
        ErroEstrutura();
        return false;
    }
    if(temp==NULL || temp->QueryDoubleAttribute("velTiro",&i_veltiro)!=XML_SUCCESS){
        ErroEstrutura();
        return false;
    }
    if(temp==NULL || temp->QueryDoubleAttribute("freqTiro",&freqTiro)!=XML_SUCCESS){
        ErroEstrutura();
        return false;
    }
    doc.Clear();
    c=svg.c_str();
    freopen(c,"r",i);
    if (doc.LoadFile(i) != XML_SUCCESS)
    {
        cout << "doc ARENA"<<endl;
        ErroEstrutura();
        cout << doc.ErrorStr() << endl;
        return false;
    }
    root=XMLHandle(&doc);
    temp=root.FirstChildElement("svg").FirstChildElement("line").ToElement();
    if(temp==NULL || temp->QueryDoubleAttribute("x1",&linha.x1)!=XML_SUCCESS
      || temp->QueryDoubleAttribute("x2",&linha.x2)!=XML_SUCCESS
       || temp->QueryDoubleAttribute("y1",&linha.y1)!=XML_SUCCESS
        || temp->QueryDoubleAttribute("y2",&linha.y2)!=XML_SUCCESS){
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
        if(temp->QueryDoubleAttribute("cx",&circulo_temp.cx)!=XML_SUCCESS
            || temp->QueryDoubleAttribute("cy",&circulo_temp.cy)!=XML_SUCCESS
                || temp->QueryDoubleAttribute("r",&circulo_temp.raio)!=XML_SUCCESS){
            
            cout<<"circulo "<<temp<<endl;
            ErroEstrutura();
            fclose(i);
            return false;
        }
        if(circulo_temp.color=="blue"){
            arena=circulo_temp;
        }else if(circulo_temp.color=="green"){
            jogador.hitbox=circulo_temp;
            jogador.x0=circulo_temp.cx;
            jogador.y0=circulo_temp.cy;
            jogador.raio_o=circulo_temp.raio;
            jogador.helice=0;
            jogador.angulo_canhao=0;
            jogador.estado=0;
            jogador.enabled=true;
        }else if(circulo_temp.color=="red"){
            nave_temp.hitbox=circulo_temp;
            nave_temp.angulo_o=graus2rad(double(rand()%361));
            nave_temp.angulo=nave_temp.angulo_o;
            nave_temp.x0=circulo_temp.cx;
            nave_temp.y0=circulo_temp.cy;
            nave_temp.raio_o=circulo_temp.raio;
            nave_temp.helice=0;
            nave_temp.vel=i_vel;
            nave_temp.veltiro=i_veltiro;
            nave_temp.angulo_canhao=0;
            nave_temp.estado=0;
            nave_temp.enabled=true;
            inimigos.push_back(nave_temp);
        }else if(circulo_temp.color=="orange"){
            nave_temp.hitbox=circulo_temp;
            nave_temp.angulo_o=graus2rad(double(rand()%361));
            nave_temp.x0=circulo_temp.cx;
            nave_temp.y0=circulo_temp.cy;
            nave_temp.raio_o=circulo_temp.raio;
            nave_temp.enabled=true;
            nave_temp.helice=0;
            nave_temp.vel=i_vel;
            nave_temp.veltiro=i_veltiro;
            nave_temp.angulo_canhao=0;
            nave_temp.estado=0;
            bases.push_back(nave_temp);
        }else {
            cout<<"Cor indefinida: "<<circulo_temp.color<<endl;
            fclose(i);
            return false;
        }     
    } 
    if(arena.raio<=0 || jogador.hitbox.raio<=0 || inimigos.size()==0 || bases.size()==0){
        cout<<"Circulos invalidos ou faltando"<<endl;
        fclose(i);
        return false;
    }
    fclose(i);
    return true;
}
