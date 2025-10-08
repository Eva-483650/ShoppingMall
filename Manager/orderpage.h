#ifndef ORDERPAGE_H
#define ORDERPAGE_H

#include <QWidget>
#include <QStandardItemModel>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QMessageBox>
#include <QHeaderView>
#include <QItemSelectionModel>
#include <QInputDialog>
#include <QPushButton>
#include <QHBoxLayout>
#include <QStyledItemDelegate>
#include <QDateTime>
#include "shoppingmanager.h"
#include "orderdetaildialog.h"

namespace Ui {
class OrderPage;
}

// 操作按钮委托类
class OrderButtonDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit OrderButtonDelegate(QObject *parent = nullptr);

    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const override;

    bool editorEvent(QEvent *event, QAbstractItemModel *model,
                     const QStyleOptionViewItem &option,
                     const QModelIndex &index) override;

    QSize sizeHint(const QStyleOptionViewItem &option,
                   const QModelIndex &index) const override;

signals:
    void detailClicked(int row);
    void statusClicked(int row);
};

class OrderPage : public QWidget
{
    Q_OBJECT

public:
    explicit OrderPage(QWidget *parent = nullptr);
    ~OrderPage();
    ShoppingManager *manager;
    void showEvent(QShowEvent *event);

private:
    Ui::OrderPage *ui;
    QStandardItemModel *orderModel;
    OrderButtonDelegate *buttonDelegate;
    bool isCached;
    int totalOrders;
    QJsonArray originalOrdersData; // 添加这个成员变量来保存原始订单数据

    void setupUI();
    void setupModel();
    void setupConnections();
    void loadOrderData();
    void updateOrderCount();
    void updateSelectionCount();
    void updateOrderDisplay(const QJsonArray &orders);
    void showOrderDetails(int row);
    void changeOrderStatus(int row);
    QString formatDateTime(const QString &dateTimeStr);
    QString buildOrderDetailsText(const QString &orderId, int userId, const QString &amount,
                                  const QString &status, const QString &createTime,
                                  const QJsonArray &orderItems);
private slots:
    void onSelectionChanged();
    void onDetailClicked(int row);
    void onStatusClicked(int row);
    void refreshOrder();
    void updateOrderStatusOnServer(const QString &orderId, const QString &newStatus, int row);
    void updateOrderStatusDisplay(int row, const QString &newStatus);
};

#endif // ORDERPAGE_H
