#include "window.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    Window window;
    window.resize(1000,1000);
    window.show();


    return app.exec();

}
