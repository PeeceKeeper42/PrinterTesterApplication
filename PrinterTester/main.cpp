#include <QtWidgets/QApplication>
#include <iostream>

#include "Window.h"

using namespace std;

int main(int argc, char *argv[])
{
    setlocale(LC_ALL, "Russian");
    
    QApplication a(argc, argv);
    Window w;
    w.show();
    return a.exec();
}
