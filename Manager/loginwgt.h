#ifndef LOGINWGT_H
#define LOGINWGT_H

#include <QWidget>
#include <QPaintEvent>
#include <QPainter>
#include <QMessageBox>
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <QJsonObject>
#include <QJsonArray>
#include <QKeyEvent>
#include <QShowEvent>
#include <QSettings>
#include <QToolButton>
#include "shoppingmanager.h"

namespace Ui {
class LoginWgt;
}

class LoginWgt : public QWidget
{
    Q_OBJECT

public:
    explicit LoginWgt(ShoppingManager *p);
    ~LoginWgt();

    ShoppingManager *client;

protected:
    void paintEvent(QPaintEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void showEvent(QShowEvent *event) override;

private:
    Ui::LoginWgt *ui;

    void setupManagerUI();
    void saveManagerCredentials(const QString &username);
    void loadSavedCredentials();

private slots:
    void loginUser();

signals:
    void signal_login(QJsonObject);
};

#endif // LOGINWGT_H
