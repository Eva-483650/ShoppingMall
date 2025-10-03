#ifndef COUPONSELECTDIALOG_H
#define COUPONSELECTDIALOG_H

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QLabel>
#include <QPushButton>
#include <QJsonArray>
#include <QJsonObject>
#include <QListWidget>

class CouponItem;
class ShoppingClient;

class CouponSelectDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CouponSelectDialog(int orderAmount, ShoppingClient* client, QWidget *parent = nullptr);
    ~CouponSelectDialog();

    int getSelectedCouponId() const { return m_selectedCouponId; }
    int getDiscountAmount() const { return m_discountAmount; }

signals:
    void couponSelected(int couponId, int discountAmount);

private slots:
    void onCouponItemSelected(int couponId, int discountAmount);
    void onConfirmClicked();
    void onCancelClicked();
    void onNoCouponClicked();

private:
    void loadUserCoupons();
    void setupUI();
    void addCouponItem(const QJsonObject& couponData);

private:
    ShoppingClient* m_client;
    int m_orderAmount;
    int m_selectedCouponId;
    int m_discountAmount;

    // UI 组件
    QScrollArea* scrollArea;
    QWidget* scrollWidget;
    QVBoxLayout* scrollLayout;
    QLabel* lab_title;
    QLabel* lab_orderAmount;
    QLabel* lab_selectedInfo;
    QPushButton* btn_confirm;
    QPushButton* btn_cancel;
    QPushButton* btn_noCoupon;

    QList<CouponItem*> m_couponItems;
};

#endif // COUPONSELECTDIALOG_H
