#ifndef RETURNDIALOG_H
#define RETURNDIALOG_H

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QTextEdit>
#include <QComboBox>
#include <QPushButton>
#include <QMessageBox>
#include <QJsonObject>
#include <QJsonArray>
#include "shoppingclient.h"  // 修改为您的客户端类

class ReturnDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ReturnDialog(const QString& orderId, const QString& orderPrice,
                          ShoppingClient* client, QWidget *parent = nullptr);

    QString getOrderId() const { return m_orderId; }
    QString getReturnReason() const { return m_returnReason; }
    QString getReturnType() const { return m_returnType; }

private slots:
    void onConfirmReturn();
    void onCancel();

private:
    void setupUI();
    void setupStyles();

    QString m_orderId;
    QString m_orderPrice;
    QString m_returnReason;
    QString m_returnType;
    ShoppingClient* m_client;  // 修改为您的客户端类型

    QLabel* m_orderLabel;
    QLabel* m_priceLabel;
    QComboBox* m_typeCombo;
    QTextEdit* m_reasonEdit;
    QPushButton* m_confirmBtn;
    QPushButton* m_cancelBtn;
};

#endif // RETURNDIALOG_H
