#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <sstream>
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


struct Linha3D
{
    string color;
    double x1, x2, y1, y2, z1, z2;
} linha;

struct Esfera
{
    string color;
    double cx, cy, raio, cz, altura;

} esfera_temp, arena;

struct Paralelepipedo
{
    double altura, largura, comprimento;
    double cx, cy, cz;
    string color;
} paral_temp;

struct Municao
{
    Esfera tiro;
    double angulo, phi;
};

struct Bomba
{
    Esfera shell;
    int start;
    double angulo, phi;
};

struct Nave
{
    Esfera hitbox;
    double x0, y0, z0; //posicao inicial
    int helice = 0;
    int estado = 0;       // parado,pista,aumenta,voando,parado
    double angulo;        //radianos em xy
    double phi;           //rand entre xy e z, em 0 esta em xy
    double angulo_canhao; //radianos em xy
    double phi_canhao;    //rad em z
    double angulo_o;      //angulo original
    double phi_o;      //angulo original
    double raio_o;
    double vel;     //multiplicador
    double veltiro; //multiplicador
    double a;       //aceleracao real
    double v;       //velocidade real
    bool enabled;
} jogador, nave_temp;

////////////global vars////////////
vector<Nave> bases;
vector<Nave> inimigos;
vector<Municao> tiros;
vector<Municao> tiros_inimigos;
vector<Bomba> bombas;
XMLDocument doc;
XMLElement *temp = NULL; //nodo  para manipular
int segmentos = 50;
double multiplicador = 1, freqTiro;
const int tock = 50, delay_tiro = 400;
int start, launch, score_total, score_atual = 0, a_tiro = 0;
bool tiro_pronto[2];
const unsigned char lose[] = "LOSE", win[] = "WIN";
const string black = "black", red = "red", yellow = "yellow", green = "green", blue = "blue", orange = "orange";
const int view_h_px = 500, view_w_px = 500;

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

void cor(string color)
{
    if (color == blue)
    {
        glColor3d(0.0, 0.0, 1.0);
    }
    else if (color == orange)
    {
        glColor3d(1.0, 0.5, 0.0);
    }
    else if (color == red)
    {
        glColor3d(1.0, 0.0, 0.0);
    }
    else if (color == green)
    {
        glColor3d(0.0, 1.0, 0.0);
    }
    else if (color == yellow)
    {
        glColor3d(1.0, 1.0, 0.0);
    }
    else
        glColor3d(0.0, 0.0, 0.0);
}

double distancia2D(Esfera a, Esfera b)
{
    return distancia2D(a.cx, a.cy, b.cx, b.cy);
}

double distancia(Esfera a, Esfera b)
{
    return distancia(a.cx, a.cy, b.cx, b.cy, a.cz, b.cz);
}

bool inbound()
{
    return distancia(jogador.hitbox, arena) <= (-jogador.hitbox.raio + arena.raio);
}

bool inbound(Esfera a)
{
    return distancia(a, arena) <= (-a.raio + arena.raio);
}

bool inbound(Nave *a)
{
    return inbound(a->hitbox);
}

bool inbound(Municao *a)
{
    return inbound(a->tiro);
}

bool inbound(Bomba *a)
{
    return inbound(a->shell);
}

bool contato(Esfera a, Esfera b)
{
    return distancia(a, b) <= (a.raio + b.raio);
}

////////////draw functions////////////
void init()
{
    glEnable(GL_DEPTH_TEST);
    glClearColor(1, 1, 1, 1);
    glClear(GL_COLOR_BUFFER_BIT);
    glMatrixMode(GL_PROJECTION);
    glMatrixMode(GL_MODELVIEW);
    glutInitWindowSize(view_w_px, view_h_px);
    glOrtho(arena.cx-arena.raio,arena.cx+arena.raio,arena.cy+arena.raio,arena.cy-arena.raio,arena.altura+1,-1);
    switch_camera(1);
    glutCreateWindow("trabalhocg");
}

void drawCilindro(Esfera circ) 
{
    double step = 2 * M_PI / segmentos, ox, oy;
    cor(circ.color);
    glBegin(GL_POLYGON);
    {
        for (double i = 2 * M_PI; i >=0; i -= step)
        {
            ox = double(circ.raio * cos(i)) + circ.cx;
            oy = double(circ.raio * sin(i)) + circ.cy;
            glVertex3d(ox, oy,circ.cz);
            glVertex3d(ox, oy,circ.cz+circ.altura);

        }
    }
    glEnd();
}

void drawEsfera(Esfera circ)
{
    double step = 2 * M_PI / segmentos, ox, oy, oz;
    cor(circ.color);
    glBegin(GL_POLYGON);
    {
        for (double i = 2 * M_PI;i >= 0; i -= step)
        {
            ox = double(circ.raio * cos(i)) + circ.cx;
            oy = double(circ.raio * sin(i)) + circ.cy;
            oz = double(circ.raio * sin(i)) + circ.cz;
            glVertex3d(ox, oy, oz);
        }
    }
    glEnd();
}

void drawLine(Linha3D l)
{
    cor(l.color);
    glBegin(GL_LINES);
    {
        glVertex3d(l.x1, l.y1,l.z1);
        glVertex3d(l.x2, l.y2,l.z2);
    }
    glEnd();
}

void drawParalelepipedo(Paralelepipedo r)
{
    cor(r.color);
    glBegin(GL_POLYGON); //xy-z
    {
        glVertex3d(r.cx - r.largura / 2, r.cy + r.comprimento / 2, r.cz - r.altura/2);
        glVertex3d(r.cx - r.largura / 2, r.cy - r.comprimento / 2, r.cz - r.altura/2);
        glVertex3d(r.cx + r.largura / 2, r.cy - r.comprimento / 2, r.cz - r.altura/2);
        glVertex3d(r.cx + r.largura / 2, r.cy + r.comprimento / 2, r.cz - r.altura/2);
    }
    glEnd();
    glBegin(GL_POLYGON); //xy+z
    {
        glVertex3d(r.cx - r.largura / 2, r.cy + r.comprimento / 2, r.cz + r.altura/2);
        glVertex3d(r.cx - r.largura / 2, r.cy - r.comprimento / 2, r.cz + r.altura/2);
        glVertex3d(r.cx + r.largura / 2, r.cy - r.comprimento / 2, r.cz + r.altura/2);
        glVertex3d(r.cx + r.largura / 2, r.cy + r.comprimento / 2, r.cz + r.altura/2);
    }
    glEnd();
    glBegin(GL_POLYGON); //yz-x
    {
        glVertex3d(r.cx - r.largura / 2, r.cy - r.comprimento / 2, r.cz + r.altura/2);
        glVertex3d(r.cx - r.largura / 2, r.cy - r.comprimento / 2, r.cz - r.altura/2);
        glVertex3d(r.cx - r.largura / 2, r.cy + r.comprimento / 2, r.cz - r.altura/2);
        glVertex3d(r.cx - r.largura / 2, r.cy + r.comprimento / 2, r.cz + r.altura/2);
    }
    glEnd();
    glBegin(GL_POLYGON); //yz+x
    {
        glVertex3d(r.cx + r.largura / 2, r.cy - r.comprimento / 2, r.cz + r.altura/2);
        glVertex3d(r.cx + r.largura / 2, r.cy - r.comprimento / 2, r.cz - r.altura/2);
        glVertex3d(r.cx + r.largura / 2, r.cy + r.comprimento / 2, r.cz - r.altura/2);
        glVertex3d(r.cx + r.largura / 2, r.cy + r.comprimento / 2, r.cz + r.altura/2);
    }
    glEnd();
    glBegin(GL_POLYGON); //xz-y
    {
        glVertex3d(r.cx - r.largura / 2, r.cy - r.comprimento / 2, r.cz + r.altura/2);
        glVertex3d(r.cx - r.largura / 2, r.cy - r.comprimento / 2, r.cz - r.altura/2);
        glVertex3d(r.cx + r.largura / 2, r.cy - r.comprimento / 2, r.cz - r.altura/2);
        glVertex3d(r.cx + r.largura / 2, r.cy - r.comprimento / 2, r.cz + r.altura/2);
    }
    glEnd();
    glBegin(GL_POLYGON); //xz+y
    {
        glVertex3d(r.cx - r.largura / 2, r.cy + r.comprimento / 2, r.cz + r.altura/2);
        glVertex3d(r.cx - r.largura / 2, r.cy + r.comprimento / 2, r.cz - r.altura/2);
        glVertex3d(r.cx + r.largura / 2, r.cy + r.comprimento / 2, r.cz - r.altura/2);
        glVertex3d(r.cx + r.largura / 2, r.cy + r.comprimento / 2, r.cz + r.altura/2);
    }
    glEnd();
    
}

bool teste(string arquivo)
{ // XML_SUCESS=0=false
    double i_vel, i_veltiro;
    string svg;
    svg = arquivo;
    if (arquivo[0] == '/' && svg.find("home") == string::npos)
    {
        svg = "." + arquivo;
    }
    else if (arquivo[0] == '~')
    {
        svg = string(getenv("HOME")) + arquivo.substr(1);
        svg = svg.substr(1);
    }
    FILE *i = fopen(svg.c_str(), "r");
    const char *c;
    svg = arquivo.substr(1);
    if (i == NULL)
    {
        i = fopen(svg.c_str(), "r");
        if (i == NULL)
        {
            cout << "FILE " << svg;
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
    if (temp == NULL)
    {
        ErroEstrutura();
        return false;
    }
    svg = temp->GetText();
    if (svg[0] == '/')
    {
        svg = '.' + svg;
    }
    else if (svg[0] == '~')
    {
        c = getenv("HOME");
        svg = svg.substr(1);
        svg = string(c) + svg;
    }
    temp = root.FirstChildElement("aplicacao").FirstChildElement("arquivoDaArena").FirstChildElement("nome").ToElement();
    if (temp == NULL)
    {
        ErroEstrutura();
        return false;
    }
    svg = svg + temp->GetText() + '.';
    temp = root.FirstChildElement("aplicacao").FirstChildElement("arquivoDaArena").FirstChildElement("tipo").ToElement();
    if (temp == NULL)
    {
        ErroEstrutura();
        return false;
    }
    svg = svg + temp->GetText();
    temp = root.FirstChildElement("aplicacao").FirstChildElement("jogador").ToElement();
    if (temp == NULL || temp->QueryDoubleAttribute("vel", &jogador.vel) != XML_SUCCESS)
    {
        ErroEstrutura();
        return false;
    }
    if (temp == NULL || temp->QueryDoubleAttribute("velTiro", &jogador.veltiro) != XML_SUCCESS)
    {
        ErroEstrutura();
        return false;
    }
    temp = root.FirstChildElement("aplicacao").FirstChildElement("inimigo").ToElement();
    if (temp == NULL || temp->QueryDoubleAttribute("vel", &i_vel) != XML_SUCCESS)
    {
        ErroEstrutura();
        return false;
    }
    if (temp == NULL || temp->QueryDoubleAttribute("velTiro", &i_veltiro) != XML_SUCCESS)
    {
        ErroEstrutura();
        return false;
    }
    if (temp == NULL || temp->QueryDoubleAttribute("freqTiro", &freqTiro) != XML_SUCCESS)
    {
        ErroEstrutura();
        return false;
    }
    doc.Clear();
    c = svg.c_str();
    cout << c << endl;
    freopen(c, "r", i);
    if (doc.LoadFile(i) != XML_SUCCESS)
    {
        cout << "doc ARENA" << endl;
        ErroEstrutura();
        cout << doc.ErrorStr() << endl;
        return false;
    }
    root = XMLHandle(&doc);
    temp = root.FirstChildElement("svg").FirstChildElement("line").ToElement();
    if (temp == NULL || temp->QueryDoubleAttribute("x1", &linha.x1) != XML_SUCCESS || temp->QueryDoubleAttribute("x2", &linha.x2) != XML_SUCCESS || temp->QueryDoubleAttribute("y1", &linha.y1) != XML_SUCCESS || temp->QueryDoubleAttribute("y2", &linha.y2) != XML_SUCCESS)
    {
        cout << "linha" << endl;
        ErroEstrutura();
        fclose(i);
        return false;
    }
    linha.z1=0;linha.z2=0;
    for (temp = root.FirstChildElement("svg").FirstChildElement("circle").ToElement(); temp != NULL; temp = temp->NextSiblingElement("circle"))
    {
        if (temp->QueryStringAttribute("fill", &c) == XML_SUCCESS)
        {
            esfera_temp.color = string(c);
        }
        else
        {
            cout << "circulo" << endl;
            ErroEstrutura();
            return false;
        }
        if (temp->QueryDoubleAttribute("cx", &esfera_temp.cx) != XML_SUCCESS || temp->QueryDoubleAttribute("cy", &esfera_temp.cy) != XML_SUCCESS || temp->QueryDoubleAttribute("r", &esfera_temp.raio) != XML_SUCCESS)
        {

            cout << "circulo " << temp << endl;
            ErroEstrutura();
            fclose(i);
            return false;
        }
        if (esfera_temp.color == blue)
        {
            arena = esfera_temp;
        }
        else if (esfera_temp.color == green)
        {
            jogador.hitbox = esfera_temp;
            jogador.x0 = esfera_temp.cx;
            jogador.y0 = esfera_temp.cy;
            jogador.z0=0;
            jogador.phi_o=0;
            jogador.raio_o = esfera_temp.raio;
            jogador.helice = 0;
            jogador.angulo_canhao = 0;
            jogador.estado = 0;
            jogador.enabled = true;
        }
        else if (esfera_temp.color == red)
        {
            nave_temp.hitbox = esfera_temp;
            nave_temp.angulo_o = graus2rad(double(rand() % 361));
            nave_temp.angulo = nave_temp.angulo_o;
            nave_temp.x0 = esfera_temp.cx;
            nave_temp.y0 = esfera_temp.cy;
            nave_temp.z0=0;
            nave_temp.phi_o=0;
            nave_temp.raio_o = esfera_temp.raio;
            nave_temp.helice = 0;
            nave_temp.vel = i_vel;
            nave_temp.veltiro = i_veltiro;
            nave_temp.angulo_canhao = 0;
            nave_temp.estado = 0;
            nave_temp.enabled = true;
            inimigos.push_back(nave_temp);
        }
        else if (esfera_temp.color == orange)
        {
            nave_temp.hitbox = esfera_temp;
            nave_temp.angulo_o = graus2rad(double(rand() % 360));
            nave_temp.x0 = esfera_temp.cx;
            nave_temp.y0 = esfera_temp.cy;
            nave_temp.z0 = 0;
            nave_temp.phi=0;
            nave_temp.raio_o = esfera_temp.raio;
            nave_temp.enabled = true;
            nave_temp.helice = 0;
            nave_temp.vel = i_vel;
            nave_temp.veltiro = i_veltiro;
            nave_temp.angulo_canhao = 0;
            nave_temp.estado = 0;
            bases.push_back(nave_temp);
        }
        else
        {
            cout << "Cor indefinida: " << esfera_temp.color << endl;
            fclose(i);
            return false;
        }
    }
    if (arena.raio <= 0 || jogador.hitbox.raio <= 0 || inimigos.size() == 0 || bases.size() == 0)
    {
        cout << "Circulos invalidos ou faltando" << endl;
        fclose(i);
        return false;
    }
    fclose(i);
    return true;
}

void display()
{
    ostringstream oss;
    oss << "SCORE: " << score_atual << " MAX: " << score_total;
    string str = oss.str();
    int pos;
    glClear(GL_COLOR_BUFFER_BIT);
    drawCilindro(arena);
    for (auto i : bases)
    {
        if (i.enabled)
        {
            drawEsfera(i.hitbox);
        }
    }
    drawLine(linha);
    for (auto i : inimigos)
    {
        if (i.enabled)
        {
            drawEs(&i);
        }
    }
    if (jogador.enabled)
    {
        for (auto i : bombas)
        {
            drawEsfera(i.shell);
        }
        for (auto i : tiros)
        {
            drawEsfera(i.tiro);
        }
        for (auto i : tiros_inimigos)
        {
            drawEsfera(i.tiro);
        }
    }
    cor(black);
    glRasterPos2i(arena.cx - 0.9 * arena.raio, arena.cy + 0.9 * arena.raio);
    for (auto i : str)
    {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_10, i);
    }
    if (jogador.enabled)
    {
        drawNave(&jogador);
        if (score_atual == score_total)
        {

            pos = -glutBitmapLength(GLUT_BITMAP_HELVETICA_18, win) / 2 + arena.cx;
            cor(yellow);
            glRasterPos2i(pos, arena.cy);
            glutBitmapString(GLUT_BITMAP_HELVETICA_18, win);
        }
    }
    else
    {
        pos = -glutBitmapLength(GLUT_BITMAP_HELVETICA_18, lose) / 2 + arena.cx;
        cor(red);
        glRasterPos2i(pos, arena.cy);
        glutBitmapString(GLUT_BITMAP_HELVETICA_18, lose);
    }
    glFlush();
    glutSwapBuffers();
}

////////////MAIN////////////
int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        cout << "Favor informar apenas o caminho do config." << endl;
        return 1;
    }
    string temp = string(argv[1]);
    if (temp[temp.size() - 1] != '/')
    {
        temp.append("/");
    }

    temp.append("config.xml");
    if (!teste(temp))
    {
        return 1;
    }
    score_total = bases.size();
    jogador.angulo_o = rad_linha(); //radianos
    jogador.angulo = jogador.angulo_o;
    jogador.angulo_canhao = 0;
    
    tiro_pronto[0] = true;
    tiro_pronto[1] = true;
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
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