#ifndef ORDERPAGE_H
#define ORDERPAGE_H

#include <QWidget>
#include <QPushButton>
#include "orderitem.h"
#include <QJsonArray>
#include <QJsonObject>
#include "./shoppingclient.h"
#include <QStringList>
#include <QListWidgetItem>
#include <QMap>
#include "paydialog.h"

namespace Ui {
    class OrderPage;
}

class OrderPage : public QWidget
{
    Q_OBJECT

public:
    explicit OrderPage(QWidget* parent = nullptr);
    void optimizeLayout();
    OrderPage(QJsonArray arr);
    ~OrderPage();
    ShoppingClient* client;
    QStringList statuslist;
    QMap<QString, QJsonObject> orderlist;
    void addItem(OrderItem* item);
    void setStatus(QString status);
    void setPrice(QString price);

private slots:
    void nextOrder();
    void backOrder();
    void payCurrentOrder();  // 付款槽函数

private:
    Ui::OrderPage* ui;
    int current;
    QPushButton* btn_pay;  // 在这里声明付款按钮，而不是在ui中

    void showEvent(QShowEvent* event);
    void getAllOrder();
    void updateOrderItems(QString ordernum);
    QString formatOrderTime(const QString& rawTime);
};

#endif // ORDERPAGE_H
