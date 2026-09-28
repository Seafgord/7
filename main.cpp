#include <QApplication>
#include "snake.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    Snake s;
    s.show();
    return app.exec();
}
