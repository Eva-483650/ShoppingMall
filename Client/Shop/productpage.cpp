#include "ProductPage.h"
#include "ui_ProductPage.h"

#include <QScreen>
#include <QApplication>
#include <QFile>
#include <QTimer>
#include <QGraphicsDropShadowEffect>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QEasingCurve>
#include <QVBoxLayout>
#include <QDebug>

QPointer<ProductPage> ProductPage::s_active = nullptr;

ProductPage::ProductPage(Product* pro, QWidget* parent)
	: QWidget(parent),
	ui(new Ui::ProductPage),
	m_product(pro)
{
	ui->setupUi(this);

	// 无边框 + 顶层透明
	setWindowFlags(Qt::FramelessWindowHint | Qt::Tool);
	setAttribute(Qt::WA_TranslucentBackground, true);
	setAttribute(Qt::WA_DeleteOnClose);
	setAttribute(Qt::WA_StyledBackground, false); // 顶层不绘制背景

	// ====== 1. 取得原顶层布局（Designer 生成的） ======
	QLayout* rootLayout = this->layout();      // 不要删除它
	if (!rootLayout) {
		// 极少情况：ui 没给你加布局（防御）
		rootLayout = new QVBoxLayout(this);
		rootLayout->setContentsMargins(0, 0, 0, 0);
	}

	// ====== 2. 如果 cardRoot 不存在则创建，并把现有项目挪进去 ======
	m_card = findChild<QWidget*>("cardRoot");
	if (!m_card) {
		QWidget* cardRoot = new QWidget(this);
		cardRoot->setObjectName("cardRoot");

		// 把 rootLayout 里的所有条目先取出来临时保存
		struct SavedItem { QLayoutItem* item; };
		QVector<SavedItem> saved;
		while (rootLayout->count() > 0) {
			QLayoutItem* it = rootLayout->takeAt(0);
			if (it) saved.push_back({ it });
		}

		// 给 cardRoot 建一个布局
		auto* cardLay = new QVBoxLayout(cardRoot);
		cardLay->setContentsMargins(24, 24, 24, 24);  // 内边距自己按需要改
		cardLay->setSpacing(16);

		// 把原来的项目按原顺序放进 cardRoot
		for (auto& s : saved) {
			if (QWidget* w = s.item->widget()) {
				cardLay->addWidget(w);
			}
			else if (QLayout* sub = s.item->layout()) {
				cardLay->addLayout(sub);
			}
			else {
				cardLay->addItem(s.item);
				continue;
			}
			// 不能 delete s.item，因为 item 里面还保存着 layout/widget；由 Qt 接管
		}

		// rootLayout 只再放一个 cardRoot
		rootLayout->setContentsMargins(0, 0, 0, 0);
		rootLayout->addWidget(cardRoot);

		m_card = cardRoot;
	}

	// ====== 3. 设置样式（仅作用在 cardRoot） ======
	setStyleSheet("");  // 顶层不画背景
	m_card->setAttribute(Qt::WA_StyledBackground, true);
	m_card->setStyleSheet(
		"#cardRoot {"
		"  background: qlineargradient(spread:pad,x1:0,y1:0,x2:1,y2:1,"
		"     stop:0   #d8cef6,"
		"     stop:0.45 #c8b6f2,"
		"     stop:1   #a98be3);"
		"  border-radius:20px;"
		"  border:1px solid #b79fe9;"
		"}"
		"QLabel#lab_name { color:#3a2b55; font-weight:700; }"
		"QLabel#lab_price { color:#6a35c7; }"
		"QLabel#lab_amount, QLabel#lab_sales { color:#4d3a73; }"
		"QLabel#lab_about {"
		"  background: rgba(255,255,255,0.95);"
		"  border:1px solid #d8c9f4;"
		"  color:#35284f;"
		"  border-radius:14px;"
		"  padding:12px 14px;"
		"}"
		"QLabel#lab_pic {"
		"  background:#fff;"
		"  border:2px solid #d0c0ec;"
		"  border-radius:18px;"
		"}"
		"QToolButton#btn_close, QToolButton#btn_fav {"
		"  background:rgba(255,255,255,0.75);"
		"  color:#3c2a60;"
		"  border:1px solid #bda4e6;"
		"}"
		"QToolButton#btn_close:hover { background:#ff5f72; color:#fff; border-color:#ff5f72; }"
		"QPushButton.actionBtn {"
		"  background: qlineargradient(x1:0,y1:0,x2:1,y2:1,"
		"              stop:0 #7c55d8, stop:1 #5c37b9);"
		"  color:#fff;"
		"  font-weight:600;"
		"}"
		"QPushButton.actionBtn:hover { background:#6943c6; }"
		"QPushButton#btn_buy {"
		"  background: qlineargradient(x1:0,y1:0,x2:1,y2:1,"
		"              stop:0 #ff9f4d, stop:1 #ff7a27);"
		"  color:#fff;"
		"}"
		"QPushButton#btn_buy:hover { background:#ff812e; }"
	);

	// ====== 4. 阴影加在 cardRoot 上（给它多留底部余量避免看成黑条） ======
	// 如果你还出现底部黑边，再把 rootLayout 的 bottomMargin 加大，如 12 或 20
	/*if (auto* oldEff = m_card->graphicsEffect())
		oldEff->deleteLater();
	auto* shadow = new QGraphicsDropShadowEffect(m_card);
	shadow->setBlurRadius(42);
	shadow->setOffset(0, 14);
	shadow->setColor(QColor(50, 30, 80, 120));
	m_card->setGraphicsEffect(shadow);*/

	// ====== 5. 初始化内容 ======
	initPage();

	// ====== 6. 信号 ======
	connect(ui->btn_cart, &QPushButton::clicked, this, &ProductPage::addProductToCart);
	connect(ui->btn_buy, &QPushButton::clicked, this, &ProductPage::buyNow);
	connect(ui->btn_closePanel, &QPushButton::clicked, this, [this] { startCloseAnimation(); });
	connect(ui->btn_close, &QToolButton::clicked, this, [this] { startCloseAnimation(); });
	connect(ui->btn_fav, &QToolButton::clicked, this, &ProductPage::toggleFavorite);
}

ProductPage::~ProductPage()
{
	delete ui;
}

ProductPage* ProductPage::showPopup(Product* pro, QWidget* hostWindow,
	bool clickMaskToClose,
	bool reuseIfExist)
{
	if (!hostWindow) hostWindow = QApplication::activeWindow();
	if (!pro) return nullptr;

	if (reuseIfExist && s_active) {
		s_active->m_product = pro;
		s_active->initPage();
		s_active->raise();
		return s_active;
	}

	auto* mask = new QWidget(hostWindow->window());
	mask->setObjectName("ProductMask");
	mask->setWindowFlags(Qt::FramelessWindowHint | Qt::Tool);
	mask->setAttribute(Qt::WA_StyledBackground, true);
	mask->setStyleSheet("QWidget#ProductMask { background:rgba(30,15,55,135); }");
	mask->setAttribute(Qt::WA_DeleteOnClose);
	mask->resize(hostWindow->window()->size());
	mask->move(hostWindow->window()->mapToGlobal(QPoint(0, 0)));
	mask->show();

	ProductPage* page = new ProductPage(pro, mask);
	page->m_mask = mask;
	page->m_host = hostWindow->window();
	page->m_clickMaskToClose = clickMaskToClose;

	mask->installEventFilter(page);
	if (page->m_host) page->m_host->installEventFilter(page);

	s_active = page;
	page->show();
	return page;
}

void ProductPage::initPage()
{
	if (!m_product) {
		ui->lab_title->setText("商品详情");
		ui->lab_name->setText("—");
		ui->lab_price->setText("￥0.00");
		ui->lab_amount->setText("库存：0");
		ui->lab_sales->setText("销量：0");
		ui->lab_about->setText("暂无数据");
		return;
	}
	ui->lab_title->setText(m_product->name);
	ui->lab_name->setText(m_product->name);
	ui->lab_price->setText(QStringLiteral("￥ %1").arg(m_product->price));
	ui->lab_amount->setText(QStringLiteral("库存：%1").arg(m_product->amount));
	ui->lab_sales->setText(QStringLiteral("销量：%1").arg(m_product->sales));
	ui->lab_about->setText(QStringLiteral("简介：%1").arg(m_product->about));
	loadImage();
}

void ProductPage::loadImage()
{
	if (!ui->lab_pic) return;
	QString path = resolveImagePath(m_product ? m_product->pictureaddress : QString());
	QPixmap pix;
	if (!pix.load(path)) {
		pix.load(":/images/products/error.jpg");
	}
	QSize tgt = ui->lab_pic->size();
	if (tgt.width() < 5) {
		ui->lab_pic->setPixmap(pix);
		QTimer::singleShot(0, this, [this] {
			#if QT_VERSION >= QT_VERSION_CHECK(6,0,0)
			QPixmap p = ui->lab_pic->pixmap(Qt::ReturnByValue);
			if (p.isNull()) return;
			#else
			const QPixmap* pPtr = ui->lab_pic->pixmap();
			if (!pPtr || pPtr->isNull()) return;
			QPixmap p = *pPtr;
			#endif
			ui->lab_pic->setPixmap(p.scaled(ui->lab_pic->size(),
				Qt::KeepAspectRatio,
				Qt::SmoothTransformation));
			});
	}
	else {
		ui->lab_pic->setPixmap(pix.scaled(tgt, Qt::KeepAspectRatio, Qt::SmoothTransformation));
	}
	ui->lab_pic->setScaledContents(false);
}

QString ProductPage::resolveImagePath(const QString& raw) const
{
	if (raw.startsWith(":/")) return raw;
	if (raw.startsWith("/images/")) return ":" + raw;
	if (raw.startsWith("images/")) return ":/" + raw;
	QString candidate = raw;
	if (candidate.isEmpty()) candidate = "error.png";
	if (!candidate.contains('.')) candidate += ".png";
	return QString(":/images/products/%1").arg(candidate);
}



/* ===== 遮罩 / 居中 / 动画 ===== */

void ProductPage::showEvent(QShowEvent* e)
{
	QWidget::showEvent(e);
	if (m_firstShow) {
		m_firstShow = false;
		if (m_mask) {
			updateMaskGeometry();
			centerInMask();
		}
		else {
			QScreen* scr = screen();
			if (!scr) scr = QGuiApplication::primaryScreen();
			QRect ag = scr->availableGeometry();
			move(ag.center() - QPoint(width() / 2, height() / 2));
		}
		buildOpenAnimation();
		buildCloseAnimation();
		startOpenAnimation();
	}
}

void ProductPage::closeEvent(QCloseEvent* e)
{
	if (m_mask) {
		m_mask->removeEventFilter(this);
		m_mask->close();
		m_mask = nullptr;
	}
	if (m_host) m_host->removeEventFilter(this);
	if (s_active == this) s_active = nullptr;
	QWidget::closeEvent(e);
}

void ProductPage::createMask(QWidget* host)
{
	Q_UNUSED(host)
		// 已在 showPopup 创建
}

void ProductPage::updateMaskGeometry()
{
	if (!m_mask || !m_host) return;
	QWidget* w = m_host->window();
	m_mask->setGeometry(QRect(w->mapToGlobal(QPoint(0, 0)), w->size()));
}

void ProductPage::centerInMask()
{
	if (!m_mask) return;
	QSize ms = m_mask->size();
	move((ms.width() - width()) / 2, (ms.height() - height()) / 2);
}

void ProductPage::buildOpenAnimation()
{
	if (m_openGroup) return;

	QRect endGeom = geometry();
	QRect startGeom = endGeom.marginsAdded(QMargins(
		endGeom.width() * 0.04,
		endGeom.height() * 0.04,
		endGeom.width() * 0.04,
		endGeom.height() * 0.04));

	auto* geo = new QPropertyAnimation(this, "geometry");
	geo->setDuration(260);
	geo->setStartValue(startGeom);
	geo->setEndValue(endGeom);
	geo->setEasingCurve(QEasingCurve::OutCubic);

	auto* opacity = new QPropertyAnimation(this, "windowOpacity");
	opacity->setDuration(220);
	opacity->setStartValue(0.0);
	opacity->setEndValue(1.0);
	opacity->setEasingCurve(QEasingCurve::OutCubic);

	if (m_mask) {
		m_mask->setWindowOpacity(0.0);
		auto* maskFade = new QPropertyAnimation(m_mask, "windowOpacity");
		maskFade->setDuration(200);
		maskFade->setStartValue(0.0);
		maskFade->setEndValue(1.0);
		maskFade->setEasingCurve(QEasingCurve::OutCubic);

		m_openGroup = new QParallelAnimationGroup(this);
		m_openGroup->addAnimation(geo);
		m_openGroup->addAnimation(opacity);
		m_openGroup->addAnimation(maskFade);
	}
	else {
		m_openGroup = new QParallelAnimationGroup(this);
		m_openGroup->addAnimation(geo);
		m_openGroup->addAnimation(opacity);
	}
}

void ProductPage::buildCloseAnimation()
{
	if (m_closeGroup) return;

	QRect startGeom = geometry();
	QRect endGeom = startGeom.marginsAdded(QMargins(
		startGeom.width() * 0.04,
		startGeom.height() * 0.04,
		startGeom.width() * 0.04,
		startGeom.height() * 0.04));

	auto* geo = new QPropertyAnimation(this, "geometry");
	geo->setDuration(200);
	geo->setStartValue(startGeom);
	geo->setEndValue(endGeom);
	geo->setEasingCurve(QEasingCurve::InCubic);

	auto* opacity = new QPropertyAnimation(this, "windowOpacity");
	opacity->setDuration(160);
	opacity->setStartValue(1.0);
	opacity->setEndValue(0.0);
	opacity->setEasingCurve(QEasingCurve::InCubic);

	if (m_mask) {
		auto* maskFade = new QPropertyAnimation(m_mask, "windowOpacity");
		maskFade->setDuration(160);
		maskFade->setStartValue(1.0);
		maskFade->setEndValue(0.0);
		maskFade->setEasingCurve(QEasingCurve::InCubic);
		m_closeGroup = new QParallelAnimationGroup(this);
		m_closeGroup->addAnimation(geo);
		m_closeGroup->addAnimation(opacity);
		m_closeGroup->addAnimation(maskFade);
	}
	else {
		m_closeGroup = new QParallelAnimationGroup(this);
		m_closeGroup->addAnimation(geo);
		m_closeGroup->addAnimation(opacity);
	}

	connect(m_closeGroup, &QParallelAnimationGroup::finished, this, [this] {
		close();
		});
}

void ProductPage::startOpenAnimation()
{
	if (m_openGroup) m_openGroup->start();
}

void ProductPage::startCloseAnimation()
{
	if (!m_closeGroup) buildCloseAnimation();
	if (m_closeGroup) m_closeGroup->start();
	else close();
}

/* ===== 交互 ===== */

bool ProductPage::eventFilter(QObject* obj, QEvent* ev)
{
	if (obj == m_mask) {
		if (ev->type() == QEvent::MouseButtonPress && m_clickMaskToClose) {
			auto* me = static_cast<QMouseEvent*>(ev);
			if (!this->geometry().contains(me->pos())) {
				startCloseAnimation();
				return true;
			}
		}
		else if (ev->type() == QEvent::Resize) {
			updateMaskGeometry();
			centerInMask();
		}
	}
	if (obj == m_host.data()) {
		if (ev->type() == QEvent::Move || ev->type() == QEvent::Resize) {
			updateMaskGeometry();
			centerInMask();
		}
	}
	return QWidget::eventFilter(obj, ev);
}

void ProductPage::keyPressEvent(QKeyEvent* event)
{
	if (event->key() == Qt::Key_Escape) {
		startCloseAnimation();
		return;
	}
	if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
		addProductToCart();
		return;
	}
	QWidget::keyPressEvent(event);
}

void ProductPage::toggleFavorite()
{
	m_fav = !m_fav;
	ui->btn_fav->setChecked(m_fav);
	ui->btn_fav->setText(m_fav ? "★" : "☆");
}

void ProductPage::addProductToCart()
{
	if (!m_product) {
		qWarning() << "addProductToCart: null product";
		return;
	}

	emit signal_addToCart(m_product);

	// 延迟提升窗口并关闭
	QTimer::singleShot(100, this, [this]() {
		raise();
		activateWindow();
		if (m_mask) {
			m_mask->raise();
			m_mask->activateWindow();
		}
		raise();
		});

	QTimer::singleShot(1500, this, [this]() {
		startCloseAnimation();
		});
}

ProductPage* ProductPage::simpleCenter(Product* pro, QWidget* host)
{
	ProductPage* page = new ProductPage(pro); // parent 设 nullptr 成独立顶层
	page->setAttribute(Qt::WA_DeleteOnClose);
	page->adjustSize();

	QWidget* ref = host ? host->window() : QApplication::activeWindow();
	if (ref) {
		QPoint gCenter = ref->mapToGlobal(ref->rect().center());
		page->move(gCenter.x() - page->width() / 2,
			gCenter.y() - page->height() / 2);
	}
	else {
		QScreen* scr = QGuiApplication::primaryScreen();
		QRect ag = scr->availableGeometry();
		page->move(ag.center() - QPoint(page->width() / 2, page->height() / 2));
	}
	page->show();
	return page;
}

void ProductPage::buyNow()
{
	qDebug() << "[BUY]" << (m_product ? m_product->name : "null");
}