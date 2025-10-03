#ifndef COUPONITEM_H
#define COUPONITEM_H

#include <QWidget>
#include <QJsonObject>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>

class CouponItem : public QWidget
{
    Q_OBJECT

public:
    explicit CouponItem(const QJsonObject& couponData, int orderAmount, QWidget *parent = nullptr);
    ~CouponItem();

    int getCouponId() const { return m_couponId; }
    QString getCouponCode() const { return m_couponCode; }
    bool isUsable() const { return m_isUsable; }
    int calculateDiscount(int orderAmount) const;

    void updateUsableStatus(int orderAmount);

signals:
    void couponSelected(int couponId, int discountAmount);

private slots:
    void onSelectClicked();

private:
    void setupUI();

private:
    int m_couponId;
    QString m_couponCode;
    QString m_couponName;
    QString m_couponType;
    double m_couponValue;
    int m_minAmount;
    int m_maxDiscount;
    bool m_isUsable;
    int m_orderAmount;
    QJsonObject m_couponData;

    // UI 组件
    QLabel* lab_couponName;
    QLabel* lab_couponDesc;
    QLabel* lab_couponValue;
    QLabel* lab_condition;
    QLabel* lab_validity;
    QPushButton* btn_select;
};

#endif // COUPONITEM_H
