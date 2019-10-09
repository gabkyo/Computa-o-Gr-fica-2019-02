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

struct Linha{
    string color;
    double x1,x2,y1,y2;
}linha;

double vel;
double veltiro;
double angulo_o;//radianos
double angulo;
double angulo_canhao;
double a;
double v;
double raio_o;
int dir[2];

struct Circulo
{
    string color;
    double cx,cy,raio;

}circulo_temp, jogador,arena ;

struct Retangulo{
    double lado,altura;
    double cx,cy;
    string color;
}retan_temp;

vector<Circulo> terrestres;
vector<Circulo> voadores;

XMLDocument doc;
XMLElement *temp = NULL; //nodo  para manipular
int segmentos=12;
int helice=0;
int estado=0; // parado,pista,aumenta,voando,parado
const int tock=250;

void init(){
    glClearColor(1, 1, 1, 1);
    glClear(GL_COLOR_BUFFER_BIT);
    glMatrixMode(GL_PROJECTION);
    glMatrixMode(GL_MODELVIEW);
    gluOrtho2D(arena.cx-arena.raio, arena.cx+arena.raio,arena.cy-arena.raio, arena.cy+arena.raio); //-y
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
    }else if(color=="yellow"){
        glColor3d(1.0,1.0,0.0);
    }else glColor3d(0.0,0.0,0.0);
}

bool teste(string arquivo)
{ // XML_SUCESS=0=false
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
    if(temp==NULL || temp->QueryDoubleAttribute("vel",&vel)!=XML_SUCCESS){
        ErroEstrutura();
        return false;
    }
    if(temp==NULL || temp->QueryDoubleAttribute("velTiro",&veltiro)!=XML_SUCCESS){
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

double distancia(double x1, double y1, double x2, double y2)
{
   return hypot(x2-x1,y2-y1);
}

double dist_linha(){
    return distancia(linha.x1,linha.y1,linha.x2,linha.y2);
}

double radianos(double x1, double x2, double y1, double y2){
    return atan2(x1*x2 + y1*y2,x1*y2 - y1*x2);
}

double rad_linha(){
    return radianos(linha.x1,linha.x2,linha.y1,linha.y2);
}


double distancia(Circulo a,Circulo b){
    return distancia(a.cx,a.cy,b.cx,b.cy);
}

bool inbound(){
    return distancia(jogador,arena)<=double(arena.raio-jogador.raio);
}

bool contato(Circulo a, Circulo b){
    return distancia(a.cx,a.cy,b.cx,b.cy)<=double(a.raio+b.raio);
}

void direcao(bool y,bool up){
    if(y && up){
        dir[1]++;
    }
    else if(!(y) && up){
        dir[0]++;
    }
    else if(y && !(up)){
        dir[1]--;
    }else dir[0]--;
    if(dir[0]==0 && dir[1]==0){
        estado=4;
    }else if(estado>2){
        estado=3;
        if(dir[0]==1 && dir[1]==1){
            angulo=M_PI/4;
        }
        if(dir[0]==-1 && dir[1]==1){
            angulo=-M_PI/4;
        }
        if(dir[0]==1 && dir[1]==-1){
            angulo=3*M_PI/4;
        }
        if(dir[0]==1 && dir[1]==1){
            angulo=-3*M_PI/4;
        }
        if(dir[0]==1 && dir[1]==0){
            angulo=M_PI/2;
        }
        if(dir[0]==0 && dir[1]==1){
            angulo=0;
        }
        if(dir[0]==-1 && dir[1]==0){
            angulo=-M_PI/2;
        }
        if(dir[0]==0 && dir[1]==-1){
            angulo=M_PI;
        }
    }
    glutPostRedisplay();
}

void raio(int value){
    double i=double(value)/1000;
    if(i<=2){
        jogador.raio=value*raio_o;
        glutTimerFunc(tock,raio,value+tock);
    }
}


void timer(int value){
    if(value==2){
        raio_o=jogador.raio;
        estado==2;
        glutTimerFunc(2000,timer,3);
        raio(1000);
    }else if(value==3){
        estado=4;
        v=vel*a*8;
    }
    glutPostRedisplay();
}




void tick(int value){
    double theta,angx,angy,tempo;
    tempo=double(value)/1000;
    if(estado>0 && estado<3){
        angy=pow(tempo,2)*a*cos(angulo)/2; //s=at^2/2
        angx=pow(tempo,2)*a*sin(angulo)/2;//cos*sen/cos=sen
        jogador.cx+=angx;
        jogador.cy+=angy;
    }else if(estado==3){
       jogador.cx+=tempo*v*sin(angulo);
       jogador.cy+=tempo*v*cos(angulo); 
    }
    glutPostRedisplay();
    glutTimerFunc(tock,tick,tock);
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

void drawJogador(){
    double theta,angx,angy;
    glPushMatrix();
    //transformado
    glTranslated(jogador.cx,jogador.cy,0);
    glRotated(-angulo*180/M_PI,0,0,1);
    glScaled(0.5,1,1);
    glTranslated(-jogador.cx,-jogador.cy,0);
    retan_temp.altura=jogador.raio/3;
    retan_temp.lado=jogador.raio*2/5;  
    drawCircle(jogador);
    circulo_temp.color="black";
    circulo_temp.raio=jogador.raio*0.4;
    circulo_temp.cx=jogador.cx;
    circulo_temp.cy=jogador.cy+circulo_temp.raio;
    drawCircle(circulo_temp);
    
    glBegin(GL_POLYGON);{//asa esquerda
        glVertex2d(jogador.cx-jogador.raio,jogador.cy-jogador.raio/3);
        glVertex2d(jogador.cx-jogador.raio,jogador.cy+jogador.raio/3);
        glVertex2d(jogador.cx-3*jogador.raio,jogador.cy);
        glVertex2d(jogador.cx-3*jogador.raio,jogador.cy-2*jogador.raio/3);
    }
    glEnd();
    glBegin(GL_POLYGON);{//asa direita
        glVertex2d(jogador.cx+jogador.raio,jogador.cy-jogador.raio/3);
        glVertex2d(jogador.cx+jogador.raio,jogador.cy+jogador.raio/3);
        glVertex2d(jogador.cx+3*jogador.raio,jogador.cy);
        glVertex2d(jogador.cx+3*jogador.raio,jogador.cy-2*jogador.raio/3);
    }
    glEnd();
    glBegin(GL_POLYGON);{//cauda
        glVertex2d(jogador.cx+jogador.raio/5,jogador.cy-jogador.raio);
        glVertex2d(jogador.cx-jogador.raio/5,jogador.cy-jogador.raio);
        glVertex2d(jogador.cx-jogador.raio/5,jogador.cy-jogador.raio/6);
        glVertex2d(jogador.cx+jogador.raio/5,jogador.cy-jogador.raio/6);
    }
    glEnd();
    angx=jogador.raio*0.5*sin(angulo_canhao);
    angy=jogador.raio*0.5*cos(angulo_canhao);
    glTranslated(jogador.cx,jogador.cy+jogador.raio,0);
    glBegin(GL_POLYGON);{//canhao
        glVertex2d(+jogador.raio/5,0);
        glVertex2d(-jogador.raio/5,0);
        glVertex2d(-jogador.raio/5+angx,angy);
        glVertex2d(+jogador.raio/5+angx,angy);
    }
    glEnd();
    glTranslated(-jogador.cx,-jogador.cy-jogador.raio,0);
    //desfazer transf
    glTranslated(jogador.cx,jogador.cy,0);
    glScaled(2,1,1);
    glRotated(+angulo*180/M_PI,0,0,1);
    glTranslated(-jogador.cx,-jogador.cy,0);
    glPopMatrix();
}

void acao(){

}

void display(){
    glClear(GL_COLOR_BUFFER_BIT);
    drawCircle(arena);
    for(auto i:terrestres){
        drawCircle(i);
    }
    for(auto i:voadores){
        drawCircle(i);
    }
    drawLine(linha);
    acao();
    drawJogador();
    glFlush();
    glutSwapBuffers(); 
}


void keyPress(unsigned char key, int x ,int y){
    if(key=='r'){
        jogador.cx=linha.x1;
        jogador.cy=linha.y1;
        angulo=angulo_o;
        estado=0;
        glutPostRedisplay();

    }else if(key=='u' && estado==0){
        estado=1;
        a=dist_linha();
        angulo_o=rad_linha(); //radianos
        angulo=angulo_o;
        angulo_canhao=0;
        glutTimerFunc(2000,timer,1);
        glutTimerFunc(tock,tick,tock);
        glutPostRedisplay();
    }
    if(key=='w' && estado>2){
        direcao(true,true);
    }
    if(key=='s' && estado>2){
        direcao(true,false);
    }
    if(key=='a' && estado>2){
        direcao(false,false);
    }
    if(key=='d' && estado>2){
        direcao(false,true);
    }
}

void keyUp(unsigned char key, int x ,int y){
    if(key=='w' && estado>2){
        direcao(false,false);
    }
    if(key=='s' && estado>2){
        direcao(false,true);
    }
    if(key=='a' && estado>2){
        direcao(true,true);
    }
    if(key=='d' && estado>2){
        direcao(true,false);
    }
}

void mouse(int button, int state,int x,int y){

}

void tracking(int x, int y){
    double t=x-arena.raio;
    angulo_canhao=(t/arena.raio)*(M_PI/4);
    glutPostRedisplay();
}

int main(int argc, char *argv[])
{
    if(argc!=2){
        cout<<"Favor informar apenas o caminho do config."<<endl;
        return 1;
    }
    string temp = string(argv[1]);
    temp.append("config.xml");
    if (!teste(temp))
    {
        return 1;
    }
    angulo_o=rad_linha(); //radianos
    angulo=angulo_o;
    angulo_canhao=0;
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(arena.raio*2, arena.raio*2);
    glutCreateWindow("trabalhocg");
    glutSetKeyRepeat(GLUT_KEY_REPEAT_OFF);
    init();
    //fim do setup
    glutKeyboardFunc(keyPress);
    glutKeyboardUpFunc(keyUp);
    glutDisplayFunc(display);
    glutMouseFunc(mouse);
    glutPassiveMotionFunc(tracking);
    glutMotionFunc(tracking);
    //fim das definicoes de estados

    glutMainLoop();

    return 0;
}
