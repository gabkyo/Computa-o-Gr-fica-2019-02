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
#include <time.h>
#include <unistd.h>

using namespace std;
using namespace tinyxml2;

int segmentos=30;
double vel, vx,vy, ro;
bool wasd[4];
int estado=0;
time_t start, now,launch; //multiplicar por CLOCKS_PER_SEC

struct Linha{
    string color;
    double x1,x2,y1,y2;
}linha;

struct Circulo
{
    string color;
    double cx,cy,raio;

}circulo_temp, jogador,arena ;

vector<Circulo> terrestres;
vector<Circulo> voadores;

XMLDocument doc;
XMLElement *temp = NULL; //nodo  para manipular

void init(){
    glClearColor(1, 1, 1, 1);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(arena.cx-arena.raio, arena.cx+arena.raio,arena.cy-arena.raio, arena.cy+arena.raio, -1, 1); //-y
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
    temp=root.FirstChildElement("aplicacao").FirstChildElement("jogador").ToElement();
    if(temp==NULL || temp->QueryDoubleAttribute("vel",&vel)!=XML_SUCCESS){
        ErroEstrutura();
        return false;
    }
    doc.Clear();
    c=svg.c_str();
    freopen(c,"r",i);
    cout<<svg<<endl;
    if (doc.LoadFile(svg.c_str()) != XML_SUCCESS)
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
   return hypot(max(x1,x2)-min(x1,x2),max(y1,y2)-min(y1,y2));
}


double distancia(Circulo a,Circulo b){
    return distancia(a.cx,a.cy,b.cx,b.cy);
}

bool inbound(){
    return distancia(jogador,arena)<=double(arena.raio-jogador.raio);
}

double convtime(time_t a, time_t b){
    return ((double) (a - b)) / CLOCKS_PER_SEC;
}

double tempo(){
    return convtime(now,start);
}

bool contato(Circulo a, Circulo b){
    return distancia(a.cx,a.cy,b.cx,b.cy)<=double(a.raio+b.raio);
}

void idle(){
    glutPostRedisplay();
}

void mover(){  // - x a +x, +y a -y
    bool act=true;
    if(convtime(now,launch)>=2 && estado==1){
        estado=2;
    }
    if(estado==3 || estado==0){
        return;
    }
    if(estado==2){
        jogador.raio=double(ro*convtime(now,launch)/2);
        if(convtime(now,launch)>=4){
            jogador.raio=double(ro*2);
            vx=vel*vx;
            vy=vel*vy;
            estado=3;
            wasd[0]=false;wasd[1]=false;wasd[2]=false;wasd[3]=false;

        }
    }
    if(estado>2 && wasd[0]==false && wasd[1]==false && wasd[2]==false && wasd[3]==false){
        estado=3;
    }
    circulo_temp=jogador;
    if(!(wasd[0] && wasd[2])){
        if(wasd[0]){
            jogador.cy+=vy*tempo();
        }
        if(wasd[2]){
            jogador.cy-=vy*tempo();
        } 
    }else {
        wasd[0]=false;wasd[2]=false;
    }
    if(!(wasd[1] && wasd[3])){
        if(wasd[3]){
            jogador.cx-=vx*tempo();
        } 
        if(wasd[1]){
            jogador.cx+=vx*tempo();
        }
    }else {
        wasd[1]=false;wasd[3]=false;
    }
    if(!inbound()){
        act=false;
        if(estado>2){
            estado=3;
            
        }
    }
    for(auto i:voadores){
        if(contato(jogador,i)){
            act=false;
            if(estado>2){
                estado=3;
            }
        }
    }
    if(!act){
        jogador=circulo_temp;
        if(estado<3){
            estado=0;
            jogador.cx=linha.x1;
            jogador.cy=linha.y1;
        }
    }
    if(estado>2 && wasd[0]==false && wasd[1]==false && wasd[2]==false && wasd[3]==false){
        estado=3;
    }
}

void keyUp(unsigned char key,int x, int y){
    if(estado<3){
        return;
    }
    if(key=='w' && estado>2){
        wasd[0]=false;
        glutPostRedisplay();
    }
    if(key=='s' && estado>2){
        wasd[2]=false;
        glutPostRedisplay();
    }
    if(key=='d' && estado>2){
        wasd[1]=false;
        glutPostRedisplay();
    }
    if(key=='a' && estado>2){
        wasd[3]=false;
        glutPostRedisplay();
    }
}

void keyPress(unsigned char key,int x, int y){
    if(key=='u' && estado==0){
        ro=jogador.raio;
        vx=double(linha.x2-linha.x1)/4;
        vy=double(linha.y2-linha.y1)/4;
        if(vx>0){
            wasd[1]=true;
        }else wasd[3]=true;
        if(vy>0){
            wasd[0]=true;
        }else wasd[2]=true;
        vx=abs(vx);
        vy=abs(vy);
        estado=1;
        start=clock();
        launch=start;
        glutPostRedisplay();
        
    }
    if(key=='w' && estado>2){
        wasd[0]=true;
        estado=4;
        glutPostRedisplay();
    }
    if(key=='s' && estado>2){
        wasd[2]=true;
        estado=4;
        glutPostRedisplay();
    }
    if(key=='d' && estado>2){
        wasd[1]=true;
        estado=4;
        glutPostRedisplay();
    }
    if(key=='a' && estado>2){
        wasd[3]=true;
        estado=4;
        glutPostRedisplay();
    }
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

void display(){
    sleep(0.2); 
    now=clock();
    mover();
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
    start=now;
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
    glutSetKeyRepeat(GLUT_KEY_REPEAT_OFF);
    init();
    //fim do setup
    glutKeyboardFunc(keyPress);
    glutKeyboardUpFunc(keyUp);
    glutDisplayFunc(display);
    glutIdleFunc(idle);
    //fim das definicoes de estados
    glutMainLoop();

    return 0;
}
