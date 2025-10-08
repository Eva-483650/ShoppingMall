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
#include <QMessageBox>
#include "paydialog.h"
#include "couponselectdialog.h"
#include "returndialog.h"

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
    void returnCurrentOrder();  // 新增退货槽函数

private:

    int current;
    QPushButton* btn_pay;  // 在这里声明付款按钮，而不是在ui中
    Ui::OrderPage* ui;
    QPushButton* btn_return;  // 新增退货按钮

    void showEvent(QShowEvent* event);
    void getAllOrder();
    void updateOrderItems(QString ordernum);
    QString formatOrderTime(const QString& rawTime);
    void updateButtonsVisibility(const QString& status);  // 新增按钮可见性控制函数
};

#endif // ORDERPAGE_H
