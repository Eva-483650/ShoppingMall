#ifndef CARTITEM_H
#define CARTITEM_H

#include <QWidget>
#include <QTimer>
#include <QListWidgetItem>

QT_BEGIN_NAMESPACE
namespace Ui { class CartItem; }
QT_END_NAMESPACE

class CartItem : public QWidget
{
	Q_OBJECT
public:
	explicit CartItem(int id,
		const QString& pictureAddress,
		const QString& name,
		int unitPrice,
		int initQty,
		QWidget* parent = nullptr);
	~CartItem() override;

	void updateTotalLabel(const QString& text);

	int  productId()     const { return m_id; }
	int  unitPrice()     const { return m_unitPrice; }
	int  currentQty()    const;
	int  confirmedQty()  const { return m_confirmedQty; }
	bool isChecked()     const;
	int  totalPrice()    const { return unitPrice() * currentQty(); }

	void setConfirmedQty(int q) { m_confirmedQty = q; }
	void setLoading(bool on);
	void setPicture(const QString& relPath);
	void setQuantity(int qty);

	QListWidgetItem* listItem = nullptr;

signals:
	void sigQuantityChanged(int proId, int newQty);
	void sigSelectionChanged(int proId, bool checked);
	void sigDeleteRequest(int proId);

public slots:
	void onSpinChanged(int value);
	void onChecked(bool c);
	void onDeleteClicked();

private:
	void setupUI();

private:
	Ui::CartItem* ui;
	int m_id;
	int m_unitPrice;
	int m_confirmedQty;
	bool m_loading = false;
};

#endif // CARTITEM_H