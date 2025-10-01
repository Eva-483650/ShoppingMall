#ifndef PAYDIALOG_H
#define PAYDIALOG_H

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QMessageBox>
#include <QScrollArea>

class PayDialog : public QDialog
{
    Q_OBJECT

public:
    PayDialog(const QString& orderId, const QString& orderPrice, const QString& userBalance, QWidget *parent = nullptr);
    ~PayDialog();

    QString getOrderId() const { return m_orderId; }
    QString getOrderPrice() const { return m_orderPrice; }

private slots:
    void confirmPayment();
    void cancelPayment();

private:
    void setupUI();

    QString m_orderId;
    QString m_orderPrice;
    QString m_userBalance;

    QLabel* lab_orderInfo;
    QLabel* lab_priceInfo;
    QLabel* lab_balanceInfo;
    QPushButton* btn_confirm;
    QPushButton* btn_cancel;
};

#endif // PAYDIALOG_H
