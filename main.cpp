#include <QApplication>
#include "src/mainwindow.h"//"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow wnd("http://localhost:3723");
    wnd.show();

    return a.exec();
}
