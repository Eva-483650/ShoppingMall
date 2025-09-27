#ifndef CARTPAGE_H
#define CARTPAGE_H

#include <QWidget>
#include <QHash>
#include <QSet>

class QListWidgetItem;
class ShoppingClient;
class Product;
class CartItem;
class QTimer;

namespace Ui { class CartPage; }

class CartPage : public QWidget
{
	Q_OBJECT
public:
	explicit CartPage(QWidget* parent = nullptr);
	~CartPage() override;

	ShoppingClient* client = nullptr;

	void resetCache();   // 登录后可调用

signals:
	void signal_updateUserMoney(int newMoney);
	void signal_updateCartMoney(int change);
	void signal_addToCartResult(bool success, const QString& message);  // 新增信号

public slots:
	void addToCart(Product* product);

private slots:
	void showEvent(QShowEvent* event) override;

	// 来自 CartItem
	void onItemQuantityChanged(int proId, int newQty);
	void onItemSelectionChanged(int proId, bool checked);
	void onItemDelete(int proId);

	// 防抖后真正提交
	void onCommitQuantity(int proId);

	void buySth();
	void updateCartMoney(int change); // 兼容旧的调用

private:
	void loadCart();
	void recalcTotal();
	void clearAll();

	void commitQuantityToServer(int proId, int newQty);   // 同步发送
	void deleteCartServer(int proId);                     // 删除(服务器)
	void removeItemLocal(int proId);                      // 本地删除
	void scheduleDebounce(int proId);                     // 启动防抖

private:
	Ui::CartPage* ui = nullptr;

	// 数据
	QHash<int, CartItem*> m_items;       // proId -> CartItem
	QHash<int, QTimer*>   m_debounce;    // proId -> timer
	QSet<int>             m_pending;     // 正在提交的 id

	bool m_cached = false;
	int  m_total = 0;
};

#endif // CARTPAGE_H