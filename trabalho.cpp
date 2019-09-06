#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <cstring>
#include <list>
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

struct Circulo
{
    double raio, rgb_circulo[3], rgb_modelo[3], sobreposicao[3];
} circulo;

struct Janela
{
    int dimensao[2];
    double fundo_rgb[3];
    string titulo;
} janela;

XMLDocument doc;
XMLElement *temp = NULL; //nodo  para manipular
list<pair<int, int>> marcados;
int mouse[2];
const double pi = 3.14;
const int segmentos = 12;
bool dragging = false;
auto pos = marcados.begin();

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
    if (i == NULL)
    {
        cout << "FILE";
        ErroIO();
        return false;
    }
    if (doc.LoadFile(i) != XML_SUCCESS)
    {
        cout << "doc";
        ErroIO();
        cout << doc.ErrorStr() << endl;
        return false;
    }
    fclose(i);
    XMLHandle root(&doc); //pointer para navegar
    temp = root.FirstChildElement("aplicacao").FirstChildElement("circulo").ToElement();
    if (temp == NULL || temp->QueryDoubleAttribute("raio", &circulo.raio) != XML_SUCCESS || temp->QueryDoubleAttribute("corR", &circulo.rgb_circulo[0]) != XML_SUCCESS || temp->QueryDoubleAttribute("corG", &circulo.rgb_circulo[1]) != XML_SUCCESS || temp->QueryDoubleAttribute("corB", &circulo.rgb_circulo[2]) != XML_SUCCESS)
    {
        ErroEstrutura();
        cout << "circulo" << endl;
        return false;
    }
    temp = root.FirstChildElement("aplicacao").FirstChildElement("circuloModelo").ToElement();
    if (temp == NULL || temp->QueryDoubleAttribute("corR", &circulo.rgb_modelo[0]) != XML_SUCCESS || temp->QueryDoubleAttribute("corG", &circulo.rgb_modelo[1]) != XML_SUCCESS || temp->QueryDoubleAttribute("corB", &circulo.rgb_modelo[2]) != XML_SUCCESS || temp->QueryDoubleAttribute("corSobreposicaoR", &circulo.sobreposicao[0]) != XML_SUCCESS || temp->QueryDoubleAttribute("corSobreposicaoG", &circulo.sobreposicao[1]) != XML_SUCCESS || temp->QueryDoubleAttribute("corSobreposicaoB", &circulo.sobreposicao[2]) != XML_SUCCESS)
    {
        ErroEstrutura();
        cout << "circuloModelo" << endl;
        return false;
    }
    temp = root.FirstChildElement("aplicacao").FirstChildElement("janela").FirstChildElement("dimensao").ToElement();
    if (temp == NULL || temp->QueryIntAttribute("largura", &janela.dimensao[0]) != XML_SUCCESS || temp->QueryIntAttribute("altura", &janela.dimensao[1]) != XML_SUCCESS)
    {
        ErroEstrutura();
        cout << "dimensao" << endl;
        return false;
    }
    temp = root.FirstChildElement("aplicacao").FirstChildElement("janela").FirstChildElement("fundo").ToElement();
    if (temp == NULL || temp->QueryDoubleAttribute("corR", &janela.fundo_rgb[0]) != XML_SUCCESS || temp->QueryDoubleAttribute("corG", &janela.fundo_rgb[1]) != XML_SUCCESS || temp->QueryDoubleAttribute("corB", &janela.fundo_rgb[2]) != XML_SUCCESS)
    {
        ErroEstrutura();
        cout << "fundo" << endl;
        return false;
    }
    temp = root.FirstChildElement("aplicacao").FirstChildElement("janela").FirstChildElement("titulo").ToElement();
    if (temp == NULL || temp->GetText() == NULL)
    {
        ErroEstrutura();
        cout << "titulo" << endl;
        return false;
    }
    janela.titulo = string(temp->GetText());
    return true;
}

double distancia(int x1, int x2, int y1, int y2)
{
    double a = double(pow((max(x1, x2) - min(x1, x2)), 2)),
           b = double(pow((max(y1, y2) - min(y1, y2)), 2));
    return sqrt(a + b);
}

bool overlap(int x, int y)
{
    for (auto const &i : marcados)
    {
        if (distancia(x, i.first, y, i.second) <= 2 * circulo.raio)
        {
            return true;
        }
    }
    return false;
}

void idle()
{
    glutPostRedisplay();
}

void click(int button, int state, int x, int y)
{
    if (button == GLUT_LEFT_BUTTON && state == GLUT_UP && !dragging)
    {
        if (!overlap(x, -y) && !dragging)
        {
            marcados.push_back(make_pair(x, -y));
        }
        idle();
    }
    if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN && !dragging){
        for (auto i = marcados.begin(); i != marcados.end(); i++)
        {
            if (distancia(x, i->first, -y, i->second) <= circulo.raio)
            {
                dragging = true;
                i->first = x;
                i->second = -y;
                pos = i;
            }
        }
    }
    if (button == GLUT_RIGHT_BUTTON && state == GLUT_UP)
    {
        if (dragging)
        {
            dragging = false;
            pos->first = x;
            pos->second = -y;
            pos=marcados.begin();
        }
        idle();
    }
}

void passivemove(int x, int y)
{
    mouse[0] = x;
    mouse[1] = -y;
    idle();
}

void drag(int x, int y)
{
    if(dragging){
        pos->first=x;
        pos->second=-y;
    }
    idle();
}

void drawCircle(int x, int y)
{
    double step = 2 * pi / segmentos, ox, oy;
    glBegin(GL_LINE_LOOP);
    {
        for (double i = 0; i <= 2 * pi; i += step)
        {
            ox = 2 * (double(x) + circulo.raio * cos(i)) / double(janela.dimensao[0]) - 1.0;
            oy = 2 * (double(y) + circulo.raio * sin(i)) / double(janela.dimensao[1]) + 1.0;
            glVertex2f(ox, oy);
        }
    }
    glEnd();
}

void display()
{
    glClear(GL_COLOR_BUFFER_BIT); //limpa tudo
    glColor3f(circulo.rgb_circulo[0], circulo.rgb_circulo[1], circulo.rgb_circulo[2]);
    for (auto const &i : marcados)
    {
        drawCircle(i.first, i.second);
    }
    if (overlap(mouse[0], mouse[1]))
    {
        glColor3f(circulo.sobreposicao[0], circulo.sobreposicao[1], circulo.sobreposicao[2]);
    }
    else
        glColor3f(circulo.rgb_modelo[0], circulo.rgb_modelo[1], circulo.rgb_modelo[2]);
    if (!dragging)
    {
        drawCircle(mouse[0], mouse[1]);
    }
    glFlush();
}

int main(int argc, char *argv[])
{
    string temp = string(argv[1]);
    temp.append("config.xml");
    if (temp[0] == '/')
    {
        temp = '.' + temp;
    }
    char arquivo[temp.length() + 1];
    strcpy(arquivo, temp.c_str());
    if (!teste(arquivo))
    {
        return 1;
    }
    glutInit(&argc, argv);
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
    //fim das definicoes de estados
    glutMainLoop();
    return 0;
}
