#include <iostream>

#include <QApplication>

#include "core.h"

int main (int argc, char** argv) {

    QApplication app (argc, argv);
    Core core;
    core.show();

    return app.exec();
}
