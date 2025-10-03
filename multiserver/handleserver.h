#ifndef HANDLESERVER_H
#define HANDLESERVER_H

#include <QObject>
#include<QJsonArray>
#include<QJsonObject>
#include<QJsonDocument>
#include<QJsonParseError>
#include"sqlserver.h"
#include"tcpserver.h"

class HandleServer:public QObject
{
    Q_OBJECT
public:
    HandleServer(SQLServer*);
    void jsonResReady(QString head,QJsonArray res,qintptr port,QString errmsg="");
    QString getRandomOrderNum();
private:
    SQLServer *sql;
    TcpServer *tcp;
    void handleUserLogin(QJsonObject body,qintptr port);
    void handleManagerLogin(QJsonObject body,qintptr port);
    void handleRegister(QJsonObject body,qintptr port);
    void handleSearchProduct(QJsonObject body,qintptr port);
    void handleAddCart(QJsonObject body,qintptr port);
    void handleDelCart(QJsonObject body,qintptr port);
    void handleUpdateCart(QJsonObject body,qintptr port);
    void handleSearchCart(QJsonObject body,qintptr port);
    void handleBuySth(QJsonObject body,qintptr port);
    bool createOrderItems(QJsonArray wannabuy,QJsonObject map,QString ordernum);
    void handleSearchOrder(QJsonObject body,qintptr port);
    void handleSearchOrderItems(QJsonObject body,qintptr port);

    // 付款逻辑
    void handlePayOrder(QJsonObject body, qintptr port);

    //优惠券逻辑
	// 在 HandleServer 类的 private 部分添加
	void handleGetUserCoupons(QJsonObject body, qintptr port);
	void handleUseCoupon(QJsonObject body, qintptr port);
	void handleGetMerchantDiscounts(QJsonObject body, qintptr port);
	void handleCalculateOrderPrice(QJsonObject body, qintptr port);

	// 辅助函数
	int calculateCouponDiscount(int couponId, int orderAmount, int userId);
	int calculateMerchantDiscount(const QJsonArray& products);
	bool validateCouponUsage(int userId, int couponId, int orderAmount);

public slots:
    void handleRequest(const QString&,const qintptr, const QByteArray);
signals:
    void signal_responeReady(const QByteArray,qintptr);
};

#endif // HANDLESERVER_H
