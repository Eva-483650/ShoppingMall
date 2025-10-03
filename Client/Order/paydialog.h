#ifndef PAYDIALOG_H
#define PAYDIALOG_H

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QMessageBox>
#include <QScrollArea>

class ShoppingClient;
class CouponSelectDialog;

class PayDialog : public QDialog
{
    Q_OBJECT

public:
    PayDialog(const QString& orderId, const QString& orderPrice, const QString& userBalance,
              ShoppingClient* client, QWidget *parent = nullptr);
    ~PayDialog();

    QString getOrderId() const { return m_orderId; }
    QString getOrderPrice() const { return m_orderPrice; }
    int getSelectedCouponId() const { return m_selectedCouponId; }
    int getCouponDiscount() const { return m_couponDiscount; }
    void queryMerchantDiscounts();
    bool useCoupon();
private slots:
    void confirmPayment();
    void cancelPayment();
    void selectCoupon();
    void onCouponSelected(int couponId, int discountAmount);

private:
    void setupUI();
    void updatePriceDisplay();
    void calculateFinalPrice();

    QString m_orderId;
    QString m_orderPrice;
    QString m_userBalance;
    ShoppingClient* m_client;

    // 优惠券相关
    int m_selectedCouponId;
    int m_couponDiscount;
    int m_merchantDiscount;
    int m_originalPrice;
    int m_finalPrice;

    QLabel* lab_orderInfo;
    QLabel* lab_originalPriceInfo;
    QLabel* lab_discountInfo;
    QLabel* lab_finalPriceInfo;
    QLabel* lab_balanceInfo;
    QLabel* lab_couponInfo;
    QPushButton* btn_confirm;
    QPushButton* btn_cancel;
    QPushButton* btn_selectCoupon;
};

#endif // PAYDIALOG_H
