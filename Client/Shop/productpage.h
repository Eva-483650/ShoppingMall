#ifndef PRODUCTPAGE_H
#define PRODUCTPAGE_H

#include <QWidget>
#include <QPointer>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>

#include "product.h"

namespace Ui { class ProductPage; }

class ProductPage : public QWidget
{
	Q_OBJECT
public:
	explicit ProductPage(Product* pro, QWidget* parent = nullptr);
	~ProductPage() override;

	static ProductPage* showPopup(Product* pro, QWidget* hostWindow,
		bool clickMaskToClose = true,
		bool reuseIfExist = true);

	int productId() const { return m_product ? m_product->id : -1; }

signals:
	void signal_addToCart(Product*);

public slots:
	void addProductToCart();
	void buyNow();
	static ProductPage* simpleCenter(Product* pro, QWidget* host);

protected:
	void showEvent(QShowEvent* e) override;
	void closeEvent(QCloseEvent* e) override;
	bool eventFilter(QObject* obj, QEvent* ev) override;
	void keyPressEvent(QKeyEvent* event) override;

private:
	Ui::ProductPage* ui;
	Product* m_product = nullptr;
	QWidget* m_card = nullptr;

	QWidget* m_mask = nullptr;
	QPointer<QWidget> m_host;
	bool m_clickMaskToClose = true;
	bool m_firstShow = true;

	QParallelAnimationGroup* m_openGroup = nullptr;
	QParallelAnimationGroup* m_closeGroup = nullptr;

	bool m_fav = false;

	void initPage();
	void loadImage();
	QString resolveImagePath(const QString& raw) const;

	void createMask(QWidget* host);
	void updateMaskGeometry();
	void centerInMask();

	void buildOpenAnimation();
	void buildCloseAnimation();
	void startOpenAnimation();
	void startCloseAnimation();

	void toggleFavorite();

	static QPointer<ProductPage> s_active;
};

#endif // PRODUCTPAGE_H