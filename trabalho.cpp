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
double multiplicador=1.0;

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

struct Municao{
    Circulo tiro;
    double angulo;
};

struct Bomba{
    Circulo shell;
    int start;
    double angulo;
};

vector<Circulo> terrestres;
vector<Circulo> voadores;
vector<Municao> tiros;
vector<Bomba> bombas;


XMLDocument doc;
XMLElement *temp = NULL; //nodo  para manipular
int segmentos=50;
int helice=0;
int estado=0; // parado,pista,aumenta,voando,parado
const int tock=50;
int start,delay[2],launch;
bool ataque[2]={true,true};

void init(){
    glClearColor(1, 1, 1, 1);
    glClear(GL_COLOR_BUFFER_BIT);
    glMatrixMode(GL_PROJECTION);
    glMatrixMode(GL_MODELVIEW);
    gluOrtho2D(arena.cx-arena.raio, arena.cx+arena.raio,arena.cy-arena.raio, arena.cy+arena.raio); //-y
}

void reset(){
    jogador.cx=linha.x1;
    jogador.cy=linha.y1;
    angulo=angulo_o;
    estado=0;
    v=0;
    a=0;
    helice=0;
    start=0;
    launch=0;
    multiplicador=1;
    jogador.raio=raio_o;
    tiros.clear();
    glutPostRedisplay();
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

double radianos(double x1, double x2, double y1, double y2){ //referencia 
    return atan2(x2-x1,y2-y1);
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

void addTiro(double ang_c,double ang_j,double x, double y, double r){
    Municao temp;
    temp.tiro.color="yellow";
    temp.tiro.raio=r/5;
    temp.angulo=ang_c+ang_j;
    temp.tiro.cx=x+r*sin(ang_j)+r*0.5*sin(ang_c);
    temp.tiro.cy=y+r*cos(ang_j)+r*0.5*cos(ang_c);
    tiros.push_back(temp);
}

void addBomba(double ang_c,double ang_j,double x, double y, double r){
    Bomba temp;
    temp.shell.color="black";
    temp.shell.raio=r/5;
    temp.angulo=ang_c+ang_j;
    temp.shell.cx=x;
    temp.shell.cy=y;
    temp.start=glutGet(GLUT_ELAPSED_TIME);
    bombas.push_back(temp);
}

void recarga(bool a){ //tiro ou bomba
    if(a){
        if(delay[0]>=8){
            addTiro(angulo_canhao,angulo,jogador.cx,jogador.cy,jogador.raio);
            delay[0]=0;
        }else delay[0]++;
    }else{
        if(delay[1]>=16){
            addBomba(angulo_canhao,angulo,jogador.cx,jogador.cy,jogador.raio);
            delay[1]=0;
        }else delay[1]++;
    }
    
    
}


void tick(int value){
    double theta,angx,angy,tempo,alfa,beta,gamma,temp;
    bool toque=false;
    if(estado==0){
        return;
    }
    if(ataque[0] && estado>2){
        recarga(true);
    }else if(ataque[1] && estado>2){
        recarga(false);
    }
    tempo=ms2s(glutGet(GLUT_ELAPSED_TIME)-value); //tempo desde ultimo passo s
    if(estado<3){
        launch+=glutGet(GLUT_ELAPSED_TIME)-value;//tempo desde lancado ms
    }else launch=0;
    if(launch>=2000 && launch<4000 && estado==1){
        estado=2;
    }else if(launch>=4000 && estado==2){
        jogador.cx=linha.x2;
        jogador.cy=linha.y2;
        v=vel*a*8*multiplicador;
        a=0;
        estado=3;
    }
    if(estado!=0 && estado!=4){
        angy=(pow(tempo,2)*a*cos(angulo)/2)+tempo*v*multiplicador*cos(angulo); //s=at^2/2
        angx=(pow(tempo,2)*a*sin(angulo)/2)+tempo*v*multiplicador*sin(angulo);//cos*sen/cos=sen
        jogador.cx+=angx;
        jogador.cy+=angy;
        for (auto i:voadores){
            if(contato(jogador,i)){
                toque=true;
            }
        }
        if(toque){
            reset();
        } 
        if(!inbound()){ 
            alfa=rad2pi(angulo);
            beta=radianos(arena.cx,jogador.cx,arena.cy,jogador.cy);
            beta=rad2pi(beta);
            if(abs(alfa-beta)<abs(beta-alfa)){
                gamma=alfa-beta;
            }else gamma=beta-alfa;
            gamma=(M_PI-2*abs(gamma));
            if(alfa>beta){
                theta=beta-gamma;
            }else theta=beta+gamma;
            jogador.cx=arena.cx+(arena.raio-jogador.raio)*sin(theta);
            jogador.cy=arena.cy+(arena.raio-jogador.raio)*cos(theta);
            for (auto i:voadores){
                if(contato(jogador,i)){
                    toque=true;
                }
            }
            if(toque){
                reset();
            }
        }
        for(auto i=tiros.begin();i!=tiros.end();){// posicao dos tiros
            (*i).tiro.cx+=v*multiplicador*tempo*sin((*i).angulo);
            (*i).tiro.cy+=v*multiplicador*tempo*cos((*i).angulo);
            if(distancia((*i).tiro,arena)>=arena.raio){
                i=tiros.erase(i);
            }else i++;
        } 
        for(auto i=bombas.begin();i!=bombas.end();){// posicao dos tiros
            temp=ms2s(glutGet(GLUT_ELAPSED_TIME)-(*i).start)/4;
            (*i).shell.cx+=v*multiplicador*tempo*sin((*i).angulo);
            (*i).shell.cy+=v*multiplicador*tempo*cos((*i).angulo);
            (*i).shell.raio=double(jogador.raio*0.5*(1-temp/2));
            if((temp==1) || (distancia((*i).shell,arena)>=arena.raio)){
                i=bombas.erase(i);
            }else i++;
        }      
        if(helice==360){
            helice=0;
        }
        helice=helice+10;
        if(estado==2){
            jogador.raio=raio_o*ms2s(launch)/2;
        }
        if(estado<3){
            v+=tempo*a;
        }
    }
    glutPostRedisplay();
    glutTimerFunc(tock,tick,glutGet(GLUT_ELAPSED_TIME));
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
    glRotated(-rad2graus(angulo),0,0,1);
    glScaled(0.33,1,1);
    glTranslated(-jogador.cx,-jogador.cy,0);
    retan_temp.altura=jogador.raio/3;
    retan_temp.lado=jogador.raio*2/5;  
    drawCircle(jogador);
    circulo_temp.color="black";
    circulo_temp.raio=jogador.raio*0.4;
    circulo_temp.cx=jogador.cx;
    circulo_temp.cy=jogador.cy+circulo_temp.raio;
    drawCircle(circulo_temp);
    glTranslated(jogador.cx,jogador.cy,0);
    
    glBegin(GL_POLYGON);{//asa esquerda
        glVertex2d(-jogador.raio,-jogador.raio/3);
        glVertex2d(-jogador.raio,+jogador.raio/3);
        glVertex2d(-3*jogador.raio,0);
        glVertex2d(-3*jogador.raio,-2*jogador.raio/3);
    }
    glEnd();
    glBegin(GL_POLYGON);{//asa direita
        glVertex2d(+jogador.raio,-jogador.raio/3);
        glVertex2d(+jogador.raio,+jogador.raio/3);
        glVertex2d(+3*jogador.raio,0);
        glVertex2d(+3*jogador.raio,-2*jogador.raio/3);
    }
    glEnd();
    glBegin(GL_POLYGON);{//cauda
        glVertex2d(+jogador.raio/5,-jogador.raio);
        glVertex2d(-jogador.raio/5,-jogador.raio);
        glVertex2d(-jogador.raio/5,-jogador.raio/6);
        glVertex2d(+jogador.raio/5,-jogador.raio/6);
    }
    glEnd();
    //helices
    cor("yellow");
    angx=jogador.raio*0.5*sqrt(2)/2;
    angy=angx;
    glTranslated(2*jogador.raio,0,0);
    glRotated(helice,0,0,1);
    glBegin(GL_POLYGON);{
        glVertex2d(0,0);
        glVertex2d(angx,angy);
        glVertex2d(angx,-angy);
        glVertex2d(-angx,angy);
        glVertex2d(-angx,-angy);
    }
    glEnd();
    glRotated(-helice,0,0,1);
    glTranslated(-4*jogador.raio,0,0);
    glRotated(helice,0,0,1);
    glBegin(GL_POLYGON);{
        glVertex2d(0,0);
        glVertex2d(angx,angy);
        glVertex2d(angx,-angy);
        glVertex2d(-angx,angy);
        glVertex2d(-angx,-angy);
    }
    glEnd();
    glRotated(-helice,0,0,1);
    glTranslated(2*jogador.raio,0,0);
    //canhao consertar
    cor("red");
    glTranslated(0,jogador.raio,0);
    glRotated(-rad2graus(angulo_canhao),0,0,1);
    glBegin(GL_POLYGON);{
        glVertex2d(+jogador.raio/5,0);
        glVertex2d(-jogador.raio/5,0);
        glVertex2d(-jogador.raio/5,jogador.raio/2);
        glVertex2d(+jogador.raio/5,jogador.raio/2);
    }
    glEnd();
    glRotated(rad2graus(angulo_canhao),0,0,1);
    glTranslated(0,-jogador.raio,0);
    //desfazer transf
    glScaled(3,1,1);
    glRotated(rad2graus(angulo),0,0,1);
    glTranslated(-jogador.cx,-jogador.cy,0);
    glPopMatrix();
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
    for(auto i:tiros){
        drawCircle(i.tiro);
    }
    for(auto i:bombas){
        drawCircle(i.shell);
    }
    drawJogador();
    glFlush();
    glutSwapBuffers(); 
}


void keyPress(unsigned char key, int x ,int y){
    if(key=='r'){
        reset();
    }else if(key=='u' && estado==0){
        estado=1;
        delay[0]=8;
        delay[1]=16;
        a=dist_linha()/8;
        angulo_o=rad_linha(); //radianos
        angulo=angulo_o;
        angulo_canhao=0;
        launch=0;
        raio_o=jogador.raio;
        ataque[0]=false;
        ataque[1]=false;
        v=0;
        start=glutGet(GLUT_ELAPSED_TIME);
        glutTimerFunc(tock,tick,start);
        glutPostRedisplay();
    }
    if(key=='a' && estado>2){
        angulo-=0.1*M_PI;
        if(angulo<-M_PI){
            angulo+=2*M_PI;
        }
    }
    if(key=='d' && estado>2){
        angulo+=0.1*M_PI;
        if(angulo>M_PI){
            angulo-=2*M_PI;
        }
    }
    if(key=='+' && estado>2){
        multiplicador+=0.2;
    }
    if(key=='-' && estado>2){
        if(multiplicador>=0.2){
            multiplicador-=0.2;
        }else multiplicador=0;
        
    }
}

void keyUp(unsigned char key, int x ,int y){
}

void mouse(int button, int state,int x,int y){
    if((button==GLUT_LEFT_BUTTON) && (state==GLUT_DOWN) && estado>2){
        ataque[0]=true;
    }else if((button==GLUT_LEFT_BUTTON) && (state==GLUT_UP) && estado>2){
        ataque[0]=false;
    }
    if((button==GLUT_RIGHT_BUTTON) && (state==GLUT_DOWN) && estado>2){
        ataque[1]=true;
    }else if((button==GLUT_RIGHT_BUTTON) && (state==GLUT_UP) && estado>2){
        ataque[1]=false;
    }
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
