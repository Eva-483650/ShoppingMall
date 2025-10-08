#ifndef ORDERDETAILDIALOG_H
#define ORDERDETAILDIALOG_H

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QFrame>
#include <QJsonArray>
#include <QJsonObject>
#include <QPropertyAnimation>
#include <QGraphicsDropShadowEffect>
#include "shoppingmanager.h"

class OrderDetailDialog : public QDialog
{
    Q_OBJECT

public:
    explicit OrderDetailDialog(const QString &orderId, int userId, const QString &amount,
                               const QString &status, const QString &createTime,
                               const QJsonArray &orderItems, ShoppingManager *manager,
                               QWidget *parent = nullptr);

private slots:
    void onCloseClicked();

private:
    void setupUI();
    void setOrderBasicInfo();
    void setOrderItems();
    void applyStyles();
    void addShadowEffect();
    void addReturnInfoSection();
    void loadReturnInfo();
    void displayReturnInfo(const QJsonArray &returnInfo);

    // UI组件
    QVBoxLayout *mainLayout;
    QScrollArea *scrollArea;
    QWidget *contentWidget;
    QVBoxLayout *contentLayout;

    QLabel *titleLabel;
    QFrame *headerFrame;
    QFrame *itemsFrame;
    QFrame *totalFrame;
    QFrame *returnFrame;
    QPushButton *closeButton;

    // 数据
    QString m_orderId;
    int m_userId;
    QString m_amount;
    QString m_status;
    QString m_createTime;
    QJsonArray m_orderItems;
    ShoppingManager *m_manager;
};

#endif // ORDERDETAILDIALOG_H
