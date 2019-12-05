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

/*
PARAMETROS
    base do trabalho 4
    jogadores e inimigos sao naves verde e vermelhos
    janela de 500x500 pix
    x e y no svg
    z da arena eh 8xdiametro FINAL do jogador = 32x raio INICIAL do jogador
    z da base =z arena (meio circulo em cima meio em baixo)
    MELHORAR CURVA PARA SER SUAVE
    BOMBA MAIOR
    INIMIGOS NAO PODEM FAZER 180
    TIROS SAEM MUITOS DE UMA VEZ REDUZIR
    NAO TELEPORTA TETO NEM CHAO

JOGADOR
    em 3d com helice e tal
    contido em uma esfera de raio no svg
    ws sobem e descem
    canhao move em 3d dependendo da posicao do mouse na tela (visto de cima: anti horario com mouse a esquerda, contrario a direita)
    canhao rotaciona max 30 graus qualquer direcao
    leftclick canhao atira
INIMIGOS
    nao tem colisao entre si
    mov aleatorio
    atirar as vezes
TIROS
    mov em 3d
    podem ser bloqueados por objetos
    resto igual
BOMBA
    igual mas em 3d
APARENCIA
    1>= luz direcional ou pontual
    modo noturno toggle com "n" qua apaga todas as luzes exceto o farol(spot) na frente do aviao apontando para a frente dele
    arena e jogador texturizados
CAMERAS
    1(default): no cockpit( bola preta) do aviao olhando para frente
    2:em cima do canhao olhando na mesma direcao com up para o ceu, vendo parte do canhao
    3:atras do jogador com distancia, acima do jogador olhado p o jogador, up para o ceu, 
    deve rotacionar de +/-180 para os lados e +/-60 para cima quando rightclick em cima da arena 
MINI MAPA
    usar camera ortogonal para fazer mapa da arena com posicoes das entidades (verde jogador, vermelho inimigos, amarelo bases)
    fundo deve ser transparente com arena representada apenas por linha
    fixo no canto inferior direito
    ocupa 1/4 da tela( 250x250 pix)
BONUS 1
    Implementar uma visão permanente da bomba. 
    câmera perspectiva no centro da bomba (up na direção do movimento). 
    janela com 200 a mais em y  constantemente (janela inicial de 500x500 será 500x700 ). DICA: É necessário dividir o viewport!
    Enquanto a bomba não for lançada, essa parte da janela reservada para ele deverá mostrar preto. 
    Quando for lançado a visão da câmera até que seja destruída, ou seja, até que ele atinja o chão.
BONUS 2
    Utilizar modelos avançados de jogador e suas partes (ver exemplos abaixo). O aluno está livre para
utilizar modelos 3D e suas partes feitos no Blender ou baixados da internet. Não pode haver grupos com
modelos repetidos. A qualidade dos modelos será julgada caso a caso. Atenção, modelos muito pesados
podem deixar o jogo muito lento e isso não é desejável.
*/

////////////STRUCTS////////////

struct Linha
{
    string color;
    double x1, x2, y1, y2;
};

struct Linha3D
{
    string color;
    double x1, x2, y1, y2, z1, z2;
} linha;

struct Circulo
{
    string color;
    double cx, cy, raio, ;

} circulo_temp;

struct Esfera
{
    string color;
    double cx, cy, raio, cz, altura;

} esfera_temp, arena;

struct Retangulo
{
    double lado, altura;
    double cx, cy;
    string color;
} retan_temp;

struct Trapezoide
{
    double altura, largura, comprimento;
    double cx, cy, cz;
    string color;
} trapez_temp;

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
    double raio_o;
    double vel;     //multiplicador
    double veltiro; //multiplicador
    double a;       //aceleracao real
    double v;       //velocidade real
    bool enabled;
} jogador, nave_temp;

////////////trigonometric////////////

double vec_mod2D(double x, double y)
{
    return sqrt(x * x + y * y);
}

double vec_mod(double x, double y, double z)
{
    return pow(x * x + y * y + z * z, 1 / 3);
}

double distancia2D(double x1, double y1, double x2, double y2)
{
    return hypot(x2 - x1, y2 - y1);
}

double distancia(double x1, double y1, double x2, double y2, double z1, double z2)
{
    return vec_mod(x2 - x1, y2 - y1, z2 - z1);
}

double dist_linha()
{
    return distancia(linha.x1, linha.y1, linha.x2, linha.y2, linha.z1, linha.z2);
}

double radianos(double x1, double x2, double y1, double y2)
{ //referencia em XY
    return atan2(x2 - x1, y2 - y1);
}

double rad_linha2D()
{
    return radianos(linha.x1, linha.x2, linha.y1, linha.y2);
}

double rad_linha_phi()
{
    return radianos(linha.x1, linha.x2, linha.z1, linha.z2);
}

int rad2graus(double rad)
{
    return rad * 180 / M_PI;
}

double graus2rad(double graus)
{
    return graus * M_PI / 180;
}

double ms2s(int ms)
{
    return ((double)ms) / 1000.0;
}

double rad2pi(double a)
{ //de 0 a 2pi
    while (a > 2 * M_PI)
    {
        a -= 2 * M_PI;
    }
    while (a < 0)
    {
        a += 2 * M_PI;
    }
    return a;
}

double angulo3d(double v1[3], double v2[3])
{
    // angulo entre vetores v1 e v2 em 3d em rad
    double v1mod, v2mod, v1n[3], v2n[3], dot;
    v1mod = sqrt(v1[0] * v1[0] + v1[1] * v1[1] + v1[2] + v1[2]);
    v2mod = sqrt(v2[0] * v2[0] + v2[1] * v2[1] + v2[2] + v2[2]);
    v1n[0] = v1[0] / v1mod;
    v1n[1] = v1[1] / v1mod;
    v1n[2] = v1[2] / v1mod;
    v2n[0] = v2[0] / v2mod;
    v2n[1] = v2[1] / v2mod;
    v2n[2] = v2[2] / v2mod;
    dot = v1n[0] * v2n[0] + v1n[1] * v2n[1] + v1n[2] * v2n[2];
    return acos(dot);
}

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

void switch_camera(int modo)
{
    double angle;
    //double angulo3d(double v1[3],double v2[3]) rad
    /*
    CAMERAS
    1(default): no cockpit( bola preta) do aviao olhando para frente
    2:em cima do canhao olhando na mesma direcao com up para o ceu, vendo parte do canhao
    3:atras do jogador com distancia, acima do jogador olhado p o jogador, up para o ceu, 
    deve rotacionar de +/-180 para os lados e +/-60 para cima quando rightclick em cima da arena 
    */
    double tx,ty,tz;
    switch (modo)
    {
    case 1:
        tx = jogador.hitbox.raio * sin(jogador.angulo);
        ty = jogador.hitbox.raio * cos(jogador.angulo);
        tz = jogador.hitbox.raio * sin(jogador.phi);
        gluLookAt(jogador.hitbox.cx + tx * 0.5, jogador.hitbox.cy + ty * 0.5, jogador.hitbox.cz + tz * 0.5, jogador.hitbox.cx + tx, jogador.hitbox.cy + ty, jogador.hitbox.cz + tz, 0, 0, cos(jogador.phi));
        break;

    default:
        break;
    }
}
////////////draw functions////////////
void init()
{
    glClearColor(1, 1, 1, 1);
    glClear(GL_COLOR_BUFFER_BIT);
    glMatrixMode(GL_PROJECTION);
    glMatrixMode(GL_MODELVIEW);
    glutInitWindowSize(view_w_px, view_h_px);
    gluPerspective(45, double(view_w_px / view_h_px), 1, arena.altura + 1);
    switch_camera(1);
}

void reset_nave(Nave *nave)
{
    nave->angulo = nave->angulo_o;
    nave->hitbox.cx = nave->x0;
    nave->hitbox.cy = nave->y0;
    nave->v = 0;
    nave->angulo_canhao = 0;
    nave->estado = 0;
    nave->a = dist_linha() / 8;
    nave->helice = 0;
    nave->enabled = true;
    nave->hitbox.raio = nave->raio_o;
}

void reset()
{
    start = 0;
    launch = 0;
    multiplicador = 1;
    a_tiro = 1;
    for (auto i = inimigos.begin(); i < inimigos.end(); ++i)
    {
        reset_nave(&(*i));
    }
    for (auto i = bases.begin(); i < bases.end(); ++i)
    {
        reset_nave(&(*i));
    }
    reset_nave(&jogador);
    tiros.clear();
    tiros_inimigos.clear();
    bombas.clear();
    score_atual = 0;
    glutPostRedisplay();
}

void drawCircle(Esfera circ)
{
    double step = 2 * M_PI / segmentos, ox, oy;
    cor(circ.color);
    glBegin(GL_POLYGON);
    {
        for (double i = 0; i <= 2 * M_PI; i += step)
        {
            ox = double(circ.raio * cos(i)) + circ.cx;
            oy = double(circ.raio * sin(i)) + circ.cy;
            glVertex2d(ox, oy);
        }
    }
    glEnd();
}

void drawSphere(Esfera circ)
{
    double step = 2 * M_PI / segmentos, ox, oy, oz;
    cor(circ.color);
    glBegin(GL_POLYGON);
    {
        for (double i = 0; i <= 2 * M_PI; i += step)
        {
            ox = double(circ.raio * cos(i)) + circ.cx;
            oy = double(circ.raio * sin(i)) + circ.cy;
            oz = double(circ.raio * sin(i)) + circ.cz;
            glVertex3d(ox, oy, oz);
        }
    }
    glEnd();
}

void drawLine(Linha l)
{
    cor(l.color);
    glBegin(GL_LINES);
    {
        glVertex2d(l.x1, l.y1);
        glVertex2d(l.x2, l.y2);
    }
    glEnd();
}

void drawRetangle(Retangulo r)
{
    cor(r.color);
    glBegin(GL_POLYGON);
    {
        glVertex2d(r.cx - r.lado / 2, r.cy - r.altura / 2);
        glVertex2d(r.cx - r.lado / 2, r.cy + r.altura / 2);
        glVertex2d(r.cx + r.lado / 2, r.cy + r.altura / 2);
        glVertex2d(r.cx + r.lado / 2, r.cy - r.altura / 2);
    }
    glEnd();
}

void drawNave(Nave *nave)
{
    double theta, angx, angy;
    glPushMatrix();
    //transformado
    glTranslated(nave->hitbox.cx, nave->hitbox.cy, 0);
    glRotated(-rad2graus(nave->angulo), 0, 0, 1);
    glScaled(0.33, 1, 1);
    glTranslated(-nave->hitbox.cx, -nave->hitbox.cy, 0);
    retan_temp.altura = nave->hitbox.raio / 3;
    retan_temp.lado = nave->hitbox.raio * 2 / 5;
    drawSphere(nave->hitbox);
    esfera_temp.color = black;
    esfera_temp.raio = nave->hitbox.raio * 0.4;
    esfera_temp.cx = nave->hitbox.cx;
    esfera_temp.cy = nave->hitbox.cy + esfera_temp.raio;
    drawSphere(esfera_temp);
    glTranslated(nave->hitbox.cx, nave->hitbox.cy, 0);
    glBegin(GL_POLYGON);
    { //asa esquerda
        glVertex2d(-nave->hitbox.raio, -nave->hitbox.raio / 3);
        glVertex2d(-nave->hitbox.raio, +nave->hitbox.raio / 3);
        glVertex2d(-3 * nave->hitbox.raio, 0);
        glVertex2d(-3 * nave->hitbox.raio, -2 * nave->hitbox.raio / 3);
    }
    glEnd();
    glBegin(GL_POLYGON);
    { //asa direita
        glVertex2d(+nave->hitbox.raio, -nave->hitbox.raio / 3);
        glVertex2d(+nave->hitbox.raio, +nave->hitbox.raio / 3);
        glVertex2d(+3 * nave->hitbox.raio, 0);
        glVertex2d(+3 * nave->hitbox.raio, -2 * nave->hitbox.raio / 3);
    }
    glEnd();
    glBegin(GL_POLYGON);
    { //cauda
        glVertex2d(+nave->hitbox.raio / 5, -nave->hitbox.raio);
        glVertex2d(-nave->hitbox.raio / 5, -nave->hitbox.raio);
        glVertex2d(-nave->hitbox.raio / 5, -nave->hitbox.raio / 6);
        glVertex2d(+nave->hitbox.raio / 5, -nave->hitbox.raio / 6);
    }
    glEnd();
    //helices
    cor(yellow);
    angx = nave->hitbox.raio * 0.5 * sqrt(2) / 2;
    angy = angx;
    glTranslated(2 * nave->hitbox.raio, 0, 0);
    glRotated(nave->helice, 0, 0, 1);
    glBegin(GL_POLYGON);
    {
        glVertex2d(0, 0);
        glVertex2d(angx, angy);
        glVertex2d(angx, -angy);
        glVertex2d(-angx, angy);
        glVertex2d(-angx, -angy);
    }
    glEnd();
    glRotated(-nave->helice, 0, 0, 1);
    glTranslated(-4 * nave->hitbox.raio, 0, 0);
    glRotated(nave->helice, 0, 0, 1);
    glBegin(GL_POLYGON);
    {
        glVertex2d(0, 0);
        glVertex2d(angx, angy);
        glVertex2d(angx, -angy);
        glVertex2d(-angx, angy);
        glVertex2d(-angx, -angy);
    }
    glEnd();
    glRotated(-nave->helice, 0, 0, 1);
    glTranslated(2 * nave->hitbox.raio, 0, 0);
    //canhao consertar
    cor(red);
    glTranslated(0, nave->hitbox.raio, 0);
    glRotated(-rad2graus(nave->angulo_canhao), 0, 0, 1);
    glBegin(GL_POLYGON);
    {
        glVertex2d(+nave->hitbox.raio / 5, 0);
        glVertex2d(-nave->hitbox.raio / 5, 0);
        glVertex2d(-nave->hitbox.raio / 5, nave->hitbox.raio / 2);
        glVertex2d(+nave->hitbox.raio / 5, nave->hitbox.raio / 2);
    }
    glEnd();
    glRotated(rad2graus(nave->angulo_canhao), 0, 0, 1);
    glTranslated(0, -nave->hitbox.raio, 0);
    //desfazer transf
    glScaled(3, 1, 1);
    glRotated(rad2graus(nave->angulo), 0, 0, 1);
    glTranslated(-nave->hitbox.cx, -nave->hitbox.cy, 0);
    glPopMatrix();
}

void addTiroInimigo(Nave *a)
{
    Municao temp;
    temp.tiro.color = red;
    temp.tiro.raio = a->hitbox.raio / 5;
    temp.angulo = a->angulo_canhao + a->angulo;
    temp.tiro.cx = a->hitbox.cx + a->hitbox.raio * sin(a->angulo) + a->hitbox.raio * 0.5 * sin(a->angulo_canhao);
    temp.tiro.cy = a->hitbox.cy + a->hitbox.raio * cos(a->angulo) + a->hitbox.raio * 0.5 * cos(a->angulo_canhao);
    tiros_inimigos.push_back(temp);
}
void addTiro()
{
    Municao temp;
    temp.tiro.color = yellow;
    temp.tiro.raio = jogador.hitbox.raio / 5;
    temp.angulo = jogador.angulo_canhao + jogador.angulo;
    temp.tiro.cx = jogador.hitbox.cx + jogador.hitbox.raio * sin(jogador.angulo) + jogador.hitbox.raio * 0.5 * sin(jogador.angulo_canhao);
    temp.tiro.cy = jogador.hitbox.cy + jogador.hitbox.raio * cos(jogador.angulo) + jogador.hitbox.raio * 0.5 * cos(jogador.angulo_canhao);
    tiros.push_back(temp);
}
void addBomba()
{
    Bomba temp;
    temp.shell.color = black;
    temp.shell.raio = jogador.hitbox.raio / 5;
    temp.angulo = jogador.angulo;
    temp.shell.cx = jogador.hitbox.cx;
    temp.shell.cy = jogador.hitbox.cy;
    temp.start = glutGet(GLUT_ELAPSED_TIME);
    bombas.push_back(temp);
}

////////////read functions////////////
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
        cout << "Esferas invalidos ou faltando" << endl;
        fclose(i);
        return false;
    }
    fclose(i);
    return true;
}
////////////timed functions////////////
void recarga_tiro(int param)
{
    tiro_pronto[0] = true;
}

void recarga_bomba(int param)
{
    tiro_pronto[1] = true;
}

void recarga_inimiga(int param)
{
    if (jogador.estado != 3)
    {
        return;
    }
    for (auto i : inimigos)
    {
        if (i.enabled)
        {
            addTiroInimigo(&i);
        }
    }
    glutTimerFunc(int(1000 * freqTiro / multiplicador), recarga_inimiga, 0);
}

void mov_inimigo(int param)
{
    if (jogador.estado != 3 || !jogador.enabled)
    {
        return;
    }
    for (auto j = inimigos.begin(); j < inimigos.end(); ++j)
    {
        if (j->enabled)
        {
            j->angulo = rand() % 360;
        }
    }
    glutTimerFunc(10 * tock / multiplicador, mov_inimigo, 0);
}

void teleport(Nave *a)
{
    double alfa, beta, theta, gamma;
    alfa = rad2pi(a->angulo);
    beta = radianos(arena.cx, a->hitbox.cx, arena.cy, a->hitbox.cy);
    beta = rad2pi(beta);
    if (abs(alfa - beta) < abs(beta - alfa))
    {
        gamma = alfa - beta;
    }
    else
        gamma = beta - alfa;
    gamma = (M_PI - 2 * abs(gamma));
    if (alfa > beta)
    {
        theta = beta - gamma;
    }
    else
        theta = beta + gamma;
    a->hitbox.cx = arena.cx + (arena.raio - a->hitbox.raio) * sin(theta);
    a->hitbox.cy = arena.cy + (arena.raio - a->hitbox.raio) * cos(theta);
}

void tick(int antigo)
{
    bool morto = false;
    int atual = glutGet(GLUT_ELAPSED_TIME);
    double dif = ms2s(atual - antigo), inicio = ms2s(atual - start), temp;
    if (jogador.enabled && jogador.estado && score_atual != score_total)
    {
        if (jogador.helice == 360)
        {
            jogador.helice = 0;
        }
        else
            jogador.helice += 10;
        jogador.v += jogador.a * dif;
        if (jogador.estado == 3)
        {
            temp = jogador.v * multiplicador * jogador.vel * dif + jogador.a * pow(dif, 2) / 2;
        }
        else
            temp = jogador.v * dif + jogador.a * pow(dif, 2) / 2;
        jogador.hitbox.cx = jogador.hitbox.cx + temp * sin(jogador.angulo);
        jogador.hitbox.cy = jogador.hitbox.cy + temp * cos(jogador.angulo);
        if (jogador.estado == 1)
        {
            if (inicio > 2)
            {
                jogador.estado = 2;
            }
        }
        if (jogador.estado == 2)
        {
            if (inicio < 4)
            {
                jogador.hitbox.raio = jogador.raio_o * (inicio / 2);
            }
            else
            {
                jogador.estado = 3;
                recarga_inimiga(0);
                glutTimerFunc(10 * tock / multiplicador, mov_inimigo, 0);
                jogador.v = jogador.a * 4 * jogador.vel;
                jogador.a = 0;
                jogador.hitbox.raio = jogador.raio_o * 2;
            }
        }
        if (!inbound())
        {
            teleport(&jogador);
        }
        if (jogador.estado == 3)
        {
            for (auto i = inimigos.begin(); i < inimigos.end(); ++i)
            {
                if (i->enabled)
                {
                    temp = jogador.v * dif * multiplicador;
                    i->hitbox.cx += temp * sin(i->angulo);
                    i->hitbox.cy += temp * cos(i->angulo);
                    if (!inbound(&(*i)))
                    {
                        teleport(&(*i));
                    }
                    if (contato(i->hitbox, jogador.hitbox))
                    {
                        morto = true;
                    }
                }
            }
        }
        for (auto i = tiros_inimigos.begin(); i < tiros_inimigos.end(); ++i)
        {
            temp = jogador.v * jogador.veltiro * dif * multiplicador;
            i->tiro.cx += (temp * sin(i->angulo));
            i->tiro.cy += (temp * cos(i->angulo));
            if (contato(i->tiro, jogador.hitbox))
            {
                morto = true;
                i = tiros_inimigos.erase(i);
            }
            else if (!inbound(&(*i)))
            {
                i = tiros_inimigos.erase(i);
            }
        }
        if (morto)
        {
            jogador.enabled = false;
        }
        else
        {
            if (jogador.estado == 3)
            {
                for (auto j = inimigos.begin(); j < inimigos.end(); ++j)
                {
                    if (j->enabled)
                    {
                        if (j->helice == 360)
                        {
                            j->helice = 0;
                        }
                        else
                            j->helice += 10;
                    }
                }
            }
            for (auto i = tiros.begin(); i < tiros.end(); ++i)
            {
                temp = jogador.v * jogador.veltiro * dif * multiplicador;
                i->tiro.cx += (temp * sin(i->angulo));
                i->tiro.cy += (temp * cos(i->angulo));
                if (!inbound(i->tiro))
                {
                    i = tiros.erase(i);
                }
                else
                {
                    for (auto j = inimigos.begin(); j < inimigos.end(); ++j)
                    {
                        if (j->enabled)
                        {
                            if (!inbound(&(*j)))
                            {
                                teleport(&(*j));
                            }
                            if (contato(i->tiro, j->hitbox))
                            {
                                j->enabled = false;
                                i = tiros.erase(i);
                            }
                        }
                    }
                }
            }
            for (auto i = bombas.begin(); i < bombas.end(); ++i)
            {
                temp = jogador.v * dif * multiplicador;
                i->shell.cy += (temp * cos(i->angulo));
                i->shell.cx += (temp * sin(i->angulo));
                if (!inbound(&(*i)) || ms2s(atual - i->start) > 2 || !(i->shell.raio > 0))
                {
                    i = bombas.erase(i);
                }
                else
                {
                    i->shell.raio = (jogador.hitbox.raio / 5) * (2 - ms2s(atual - i->start)) / 2;
                }
                for (auto j = bases.begin(); j < bases.end(); ++j)
                {
                    if (j->enabled)
                    {
                        if (!inbound(&(*j)))
                        {
                            teleport(&(*j));
                        }
                        if (contato(i->shell, j->hitbox) && ms2s(atual - i->start) > 1)
                        {
                            j->enabled = false;
                            score_atual++;
                            i = bombas.erase(i);
                        }
                    }
                }
            }
        }
        glutTimerFunc(tock, tick, atual);
        glutPostRedisplay();
    }
}

void display()
{
    ostringstream oss;
    oss << "SCORE: " << score_atual << " MAX: " << score_total;
    string str = oss.str();
    int pos;
    glClear(GL_COLOR_BUFFER_BIT);
    drawSphere(arena);
    for (auto i : bases)
    {
        if (i.enabled)
        {
            drawSphere(i.hitbox);
        }
    }
    drawLine(linha);
    for (auto i : inimigos)
    {
        if (i.enabled)
        {
            drawNave(&i);
        }
    }
    if (jogador.enabled)
    {
        for (auto i : bombas)
        {
            drawSphere(i.shell);
        }
        for (auto i : tiros)
        {
            drawSphere(i.tiro);
        }
        for (auto i : tiros_inimigos)
        {
            drawSphere(i.tiro);
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
////////////key functions////////////
void keyPress(unsigned char key, int x, int y)
{
    if (key == 'r')
    {
        reset();
    }
    else if (key == 'u' && jogador.estado == 0)
    {
        jogador.a = dist_linha() / 8;
        jogador.angulo = jogador.angulo_o;
        jogador.angulo_canhao = 0;
        launch = 0;
        jogador.v = 0;
        jogador.estado = 1;
        start = glutGet(GLUT_ELAPSED_TIME);
        tick(start);
        glutPostRedisplay();
    }
    if (key == 'd' && jogador.estado > 2 && score_atual != score_total)
    {
        jogador.angulo -= 0.1 * M_PI;
        if (jogador.angulo < -M_PI)
        {
            jogador.angulo += 2 * M_PI;
        }
    }
    if (key == 'a' && jogador.estado > 2 && score_atual != score_total)
    {
        jogador.angulo += 0.1 * M_PI;
        if (jogador.angulo > M_PI)
        {
            jogador.angulo -= 2 * M_PI;
        }
    }
    if (key == '+' && jogador.estado > 2)
    {
        multiplicador += 0.2;
    }
    if (key == '-' && jogador.estado > 2)
    {
        if (multiplicador >= 0.2)
        {
            multiplicador -= 0.2;
        }
        else
            multiplicador = 0;
    }
}
void keyUp(unsigned char key, int x, int y)
{
}

void mouse(int button, int state, int x, int y)
{
    if ((button == GLUT_LEFT_BUTTON) && (state == GLUT_DOWN) && jogador.estado > 2)
    {
        if (tiro_pronto[0])
        {
            addTiro();
            tiro_pronto[0] = false;
            glutTimerFunc(delay_tiro - a_tiro, recarga_tiro, 0);
        }
        else
        {
            if (delay_tiro - a_tiro > 100)
            {
                a_tiro++;
            }
        }
        //tiro
    }
    else if ((button == GLUT_LEFT_BUTTON) && (state == GLUT_UP) && jogador.estado > 2)
    {
        a_tiro = 0;
        //pare de atirar
    }
    if ((button == GLUT_RIGHT_BUTTON) && (state == GLUT_DOWN) && jogador.estado > 2)
    {
        if (tiro_pronto[1])
        {
            addBomba();
            tiro_pronto[1] = false;
            glutTimerFunc(delay_tiro * 2, recarga_bomba, 0);
        }
        //bomba
    }
}

void tracking(int x, int y)
{
    double t = x - arena.raio;
    jogador.angulo_canhao = (t / arena.raio) * (M_PI / 4);
    glutPostRedisplay();
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
    jogador.angulo_o = rad_linha2D(); //radianos
    jogador.angulo = jogador.angulo_o;
    jogador.angulo_canhao = 0;
    
    tiro_pronto[0] = true;
    tiro_pronto[1] = true;
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(arena.raio * 2, arena.raio * 2);
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