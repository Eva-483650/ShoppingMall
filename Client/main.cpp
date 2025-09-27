#include "shoppingclient.h"
#include "loginwgt.h"

#include <QApplication>
#include <QLocale>
#include <QTranslator>
#include <QDir>
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QDir::setCurrent(qApp->applicationDirPath());
    // 创建主窗口(ShoppingClient)
    ShoppingClient w; 
    // 创建并显示登录窗口(LoginWgt)
    LoginWgt q(&w); 
    q.show();
    return a.exec();
}
