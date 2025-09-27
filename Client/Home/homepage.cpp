#include "HomePage.h"
#include "ui_homepage.h"
#include "shoppingclient.h"

#include <QTimer>
#include <QLabel>
#include <QToolButton>
#include <QScrollArea>
#include <QListWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGraphicsDropShadowEffect>
#include <QPropertyAnimation>
#include <QDateTime>
#include <QScrollBar>
#include <QPainter>
#include <QPainterPath>
#include <QWheelEvent>
#include <QDebug>

/* =========================================================
 * 可集中调控的视觉/尺寸常量（后续统一修改方便）
 * ========================================================= */
namespace {

    constexpr int  kCarouselMinWidth = 700;   // 轮播区域最小宽（窗口小于时保持）
    constexpr double kCarouselLeftRatio = 0.62; // 轮播区域目标占整体宽度比例

    // 精选推荐卡片尺寸
    constexpr int  kCardWidth = 180;
    constexpr int  kCardHeight = 250;
    constexpr int  kImageSize = 160;
    constexpr int  kCardRadius = 18;
    constexpr int  kImageRadius = 14;

    // 自动横向滚动速度（步长 & 间隔）
    constexpr int  kAutoScrollIntervalMs = 30;  // 定时器间隔
    constexpr int  kAutoScrollStepPx = 1;   // 每次增量

    // 轮播淡入/动画（如果后续加动画可用）
} // namespace

/* =========================================================
 * 构造 / 析构
 * ========================================================= */
HomePage::HomePage(QWidget* parent)
    : QWidget(parent),
    ui(new Ui::HomePage)
{
    ui->setupUi(this);
    initUi();
    initSignals();
    initCarousel();
}

HomePage::~HomePage()
{
    stopCarouselAutoPlay();
    stopAutoScrollRecommend();
    delete ui;
}

/* =========================================================
 * 对外接口
 * ========================================================= */
void HomePage::setClient(ShoppingClient* client)
{
    m_client = client;
    loadInitialData();
}

void HomePage::loadInitialData()
{
    /* ---------- 轮播图初始化 ---------- */
    clearCarousel();

    // 这里示例加载多张（资源路径需要在 qrc 中存在）
    QStringList imgs = {
        ":/images/picturewall/1.jpg",
        ":/images/picturewall/2.jpg",
        ":/images/picturewall/3.jpg",
        ":/images/picturewall/4.jpg",
        ":/images/picturewall/5.jpg",
        ":/images/picturewall/6.jpg",
        ":/images/picturewall/7.jpg",
        ":/images/picturewall/8.jpg",
        ":/images/picturewall/9.jpg",
        ":/images/picturewall/10.jpg"
    };
    for (const auto& p : imgs)
        addCarouselSlide(p);

    rebuildDots();
    if (ui->stack_carousel->count() > 0)
        ui->stack_carousel->setCurrentIndex(0);

    /* ---------- 精选推荐 & 公告演示数据 ---------- */
    addSampleData();

    /* ---------- 初始化轮播控制 ---------- */
    initCarousel();

    /* ---------- 启动精选推荐自动横向滚动（可选） ---------- */
    startAutoScrollRecommend();

    /* ---------- 首帧后再统一调整一次尺寸（确保布局完成） ---------- */
    QTimer::singleShot(0, this, [this] {
        rescaleCarousel();
        updateRecommendWidth();
        adjustRecommendHeight();
        });
}

/* =========================================================
 * UI 初始化
 * ========================================================= */
void HomePage::initUi()
{
    // 阴影快速附着器
    auto addShadow = [](QWidget* w, int blur, QPoint offset, QColor color) {
        if (!w) return;
        auto eff = new QGraphicsDropShadowEffect(w);
        eff->setBlurRadius(blur);
        eff->setOffset(offset);
        eff->setColor(color);
        w->setGraphicsEffect(eff);
        };
    addShadow(ui->carouselFrame, 32, { 0,10 }, QColor(80, 60, 110, 90));
    addShadow(ui->frame_recommend, 28, { 0,8 }, QColor(80, 60, 110, 80));
    addShadow(ui->frame_notice, 28, { 0,8 }, QColor(80, 60, 110, 80));

    // 推荐滚动区配置：只需要横向（手动 + 自动）
    ui->scroll_recommend->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->scroll_recommend->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    ui->scroll_recommend->setWidgetResizable(false);       // 关键：不让内容被强行拉伸到视口宽
    ui->scroll_recommend->setFrameShape(QFrame::NoFrame);
    ui->scroll_recommend->viewport()->setStyleSheet("background:transparent;");
    ui->scroll_recommend->viewport()->installEventFilter(this);

    // 中间主布局左右拉伸比例
    if (ui->midMainLayout) {
        ui->midMainLayout->setStretch(0, 7);
        ui->midMainLayout->setStretch(1, 4);
    }

    ui->carouselFrame->setMinimumWidth(kCarouselMinWidth);
}

/* =========================================================
 * 信号绑定
 * ========================================================= */
void HomePage::initSignals()
{
    // 上一张
    connect(ui->btn_prev, &QToolButton::clicked, this, [this] {
        if (!ui->stack_carousel || ui->stack_carousel->count() == 0) return;
        int idx = ui->stack_carousel->currentIndex();
        idx = (idx - 1 + ui->stack_carousel->count()) % ui->stack_carousel->count();
        ui->stack_carousel->setCurrentIndex(idx);
        updateDots(idx);
        });

    // 下一张
    connect(ui->btn_next, &QToolButton::clicked, this, [this] {
        if (!ui->stack_carousel || ui->stack_carousel->count() == 0) return;
        int idx = (ui->stack_carousel->currentIndex() + 1) % ui->stack_carousel->count();
        ui->stack_carousel->setCurrentIndex(idx);
        updateDots(idx);
        });

    // 悬停轮播暂停
    ui->carouselFrame->installEventFilter(this);
}

/* =========================================================
 * 轮播初始化
 * ========================================================= */
void HomePage::initCarousel()
{
    if (!ui->stack_carousel) return;
    updateDots(ui->stack_carousel->currentIndex());
    startCarouselAutoPlay();
}

/* =========================================================
 * 轮播：添加 / 清空 / 缩放 / 指示点
 * ========================================================= */
void HomePage::addCarouselSlide(const QString& path)
{
    if (!ui->stack_carousel) return;

    QLabel* lab = new QLabel;
    lab->setAlignment(Qt::AlignCenter);
    lab->setObjectName("carouselSlide");
    lab->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    // 不使用 setScaledContents(true) → 我们在 rescaleCarousel 里手动保持等比，避免后续父窗口压缩变形
    lab->setScaledContents(false);

    QPixmap pix(path);
    if (pix.isNull()) {
        lab->setText("加载失败");
        qDebug() << "[Carousel] load fail:" << path;
    }
    else {
        lab->setPixmap(pix); // 初始先放原图（首帧后重新缩放）
    }

    ui->stack_carousel->addWidget(lab);
    m_slides.push_back({ lab, pix });
}

void HomePage::clearCarousel()
{
    if (!ui->stack_carousel) return;
    while (ui->stack_carousel->count() > 0) {
        QWidget* w = ui->stack_carousel->widget(0);
        ui->stack_carousel->removeWidget(w);
        w->deleteLater();
    }
    m_slides.clear();
    m_indicatorDots.clear();
}

void HomePage::rescaleCarousel()
{
    if (!ui->stack_carousel || m_slides.isEmpty()) return;

    QSize area = ui->stack_carousel->size();
    if (area.width() < 20 || area.height() < 20) return;

    // 留一些内边距（可结合外层布局 margin 调整）
    const int padW = 24;
    const int padH = 24;
    QSize target(area.width() - padW, area.height() - padH);
    if (target.width() <= 0 || target.height() <= 0) return;

    for (auto& s : m_slides) {
        if (!s.label || s.original.isNull()) continue;
        QPixmap scaled = s.original.scaled(target, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        s.label->setPixmap(scaled);
    }
}

void HomePage::rebuildDots()
{
    // 删除旧指示点
    for (auto* d : m_indicatorDots)
        if (d) d->deleteLater();
    m_indicatorDots.clear();

    if (!ui->indicatorLayout || !ui->stack_carousel) return;

    int pages = ui->stack_carousel->count();
    for (int i = 0; i < pages; ++i) {
        QLabel* dot = new QLabel;
        dot->setFixedSize(14, 14);
        dot->setObjectName("indicatorDot");
        dot->setProperty("current", false);
        ui->indicatorLayout->addWidget(dot);
        m_indicatorDots.push_back(dot);
    }
    updateDots(ui->stack_carousel->currentIndex());
}

void HomePage::updateDots(int activeIndex)
{
    for (int i = 0; i < m_indicatorDots.size(); ++i) {
        QLabel* dot = m_indicatorDots[i];
        if (!dot) continue;
        dot->setProperty("current", i == activeIndex);
        dot->style()->unpolish(dot);
        dot->style()->polish(dot);
    }
}

/* =========================================================
 * 轮播自动播放
 * ========================================================= */
void HomePage::startCarouselAutoPlay()
{
    if (!ui->stack_carousel) return;
    if (!m_carouselTimer) {
        m_carouselTimer = new QTimer(this);
        connect(m_carouselTimer, &QTimer::timeout, this, [this] {
            if (!ui->stack_carousel || ui->stack_carousel->count() == 0) return;
            int idx = (ui->stack_carousel->currentIndex() + 1) % ui->stack_carousel->count();
            ui->stack_carousel->setCurrentIndex(idx);
            updateDots(idx);
            });
    }
    m_carouselTimer->start(m_carouselIntervalMs);
}

void HomePage::stopCarouselAutoPlay()
{
    if (m_carouselTimer) m_carouselTimer->stop();
}

/* =========================================================
 * 精选推荐：卡片生成
 * ========================================================= */
QWidget* HomePage::makeProductCardWidget(const QString& name,
    const QString& price,
    const QPixmap& pix,
    const QString& subTitle)
{
    // 自绘图片容器（保持比例 contain）
    class ImageBox : public QWidget {
    public:
        QPixmap src, scaled;
        void setPixmap(const QPixmap& p) { src = p; updateScaled(); }
        void resizeEvent(QResizeEvent*) override { updateScaled(); }
        void updateScaled() {
            if (src.isNull()) { scaled = QPixmap(); return; }
            scaled = src.scaled(size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        }
        void paintEvent(QPaintEvent*) override {
            QPainter p(this);
            p.setRenderHint(QPainter::Antialiasing);
            QPainterPath path;
            path.addRoundedRect(rect(), kImageRadius, kImageRadius);
            p.setClipPath(path);
            p.fillRect(rect(), QColor(255, 255, 255, 110)); // 背景防止透明锯齿
            if (!scaled.isNull()) {
                QPoint tl((width() - scaled.width()) / 2,
                    (height() - scaled.height()) / 2);
                p.drawPixmap(tl, scaled);
            }
        }
    };

    auto* frame = new QFrame;
    frame->setProperty("class", "productCard");
    frame->setFixedSize(kCardWidth, kCardHeight);

    auto* v = new QVBoxLayout(frame);
    v->setContentsMargins(10, 10, 10, 10);
    v->setSpacing(6);

    auto* imgBox = new ImageBox;
    imgBox->setFixedSize(kImageSize, kImageSize);
    if (!pix.isNull())
        imgBox->setPixmap(pix);
    else {
        // 占位图
        QPixmap ph(kImageSize, kImageSize);
        ph.fill(Qt::transparent);
        QPainter pp(&ph);
        pp.setRenderHint(QPainter::Antialiasing);
        QPainterPath phPath;
        phPath.addRoundedRect(ph.rect(), kImageRadius, kImageRadius);
        pp.setClipPath(phPath);
        pp.fillRect(ph.rect(), QColor(255, 255, 255, 70));
        pp.setPen(QPen(QColor(170, 150, 200), 2, Qt::DashLine));
        pp.drawRoundedRect(ph.rect().adjusted(3, 3, -3, -3), kImageRadius - 3, kImageRadius - 3);
        imgBox->setPixmap(ph);
    }

    QLabel* labName = new QLabel(name);
    labName->setProperty("class", "productName");
    labName->setWordWrap(true);
    labName->setAlignment(Qt::AlignHCenter | Qt::AlignTop);
    labName->setMinimumHeight(32);   // 约 2 行的空间
    labName->setMaximumHeight(40);
    labName->setStyleSheet("line-height:140%;");

    QLabel* labSub = nullptr;
    if (!subTitle.isEmpty()) {
        labSub = new QLabel(subTitle);
        labSub->setAlignment(Qt::AlignCenter);
        labSub->setStyleSheet("font:10pt '微软雅黑';color:#7a6a95;");
    }

    QLabel* labPrice = new QLabel(price);
    labPrice->setProperty("class", "productPrice");
    labPrice->setAlignment(Qt::AlignCenter);

    // 排列
    v->addWidget(imgBox, 0, Qt::AlignHCenter);
    v->addSpacing(4);
    v->addWidget(labName);
    if (labSub) v->addWidget(labSub);
    v->addWidget(labPrice);
    v->addStretch(); // 让底部留呼吸空间

    // hover 动画需要事件过滤
    frame->installEventFilter(this);
    return frame;
}

/* =========================================================
 * 精选推荐：添加卡片
 * ========================================================= */
void HomePage::addProductCard(const QString& name,
    const QString& price,
    const QPixmap& pix,
    const QString& subTitle)
{
    if (!ui->recommendFlowLayout) return;
    QWidget* card = makeProductCardWidget(name, price, pix, subTitle);

    // 你原来末尾有一个 spacer，可按“spacer 前插入”逻辑处理
    int insertPos = ui->recommendFlowLayout->count() - 1;
    if (insertPos < 0) insertPos = 0;
    ui->recommendFlowLayout->insertWidget(insertPos, card);

    updateRecommendWidth();
    adjustRecommendHeight();
}

/* =========================================================
 * 精选推荐：宽度 / 高度调整
 * ========================================================= */
void HomePage::updateRecommendWidth()
{
    if (!ui->recommendFlowLayout || !ui->scroll_recommend) return;
    QWidget* content = ui->recommendFlowLayout->parentWidget(); // scrollAreaWidgetContents
    if (!content) return;

    int cardCount = 0;
    for (int i = 0; i < ui->recommendFlowLayout->count(); ++i) {
        QLayoutItem* it = ui->recommendFlowLayout->itemAt(i);
        if (!it) continue;
        if (it->spacerItem()) continue;
        if (auto* w = it->widget())
            if (w->property("class") == "productCard")
                ++cardCount;
    }

    if (cardCount == 0) {
        content->setFixedWidth(ui->scroll_recommend->viewport()->width());
        return;
    }

    int spacing = ui->recommendFlowLayout->spacing();
    QMargins mg = ui->recommendFlowLayout->contentsMargins();
    int totalW = cardCount * kCardWidth + (cardCount - 1) * spacing + mg.left() + mg.right();

    // 不够宽时多 +1 防止 AsNeeded 判定为不显示滚动条
    if (totalW < ui->scroll_recommend->viewport()->width())
        totalW = ui->scroll_recommend->viewport()->width() + 1;

    content->setFixedWidth(totalW);
    content->setFixedHeight(kCardHeight + mg.top() + mg.bottom());
}

void HomePage::adjustRecommendHeight()
{
    if (!ui->frame_recommend) return;
    int titleH = ui->lab_recommendTitle->sizeHint().height();
    int spacing = ui->recommendLayout->spacing();
    int margins = ui->recommendLayout->contentsMargins().top()
        + ui->recommendLayout->contentsMargins().bottom();

    // frame 高度 = 标题 + spacing + 卡片高度 + 外布局上下边距 + 余量
    int total = titleH + spacing + kCardHeight + margins + 6;
    ui->frame_recommend->setFixedHeight(total);
}

/* =========================================================
 * 精选推荐：自动横向滚动
 * ========================================================= */
void HomePage::startAutoScrollRecommend()
{
    if (!ui->scroll_recommend) return;
    // 若希望仍显示滚动条，注释掉下一行：
    ui->scroll_recommend->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    if (!m_recommendAutoTimer) {
        m_recommendAutoTimer = new QTimer(this);
        m_recommendAutoTimer->setInterval(kAutoScrollIntervalMs);
        connect(m_recommendAutoTimer, &QTimer::timeout,
            this, &HomePage::tickAutoScrollRecommend);
    }
    m_recommendAutoTimer->start();
}

void HomePage::stopAutoScrollRecommend()
{
    if (m_recommendAutoTimer) m_recommendAutoTimer->stop();
}

void HomePage::tickAutoScrollRecommend()
{
    if (!ui->scroll_recommend) return;
    if (m_autoScrollPaused) return;

    QScrollBar* hsb = ui->scroll_recommend->horizontalScrollBar();
    if (!hsb) return;
    int next = hsb->value() + kAutoScrollStepPx;
    if (next >= hsb->maximum()) {
        // 循环（若要无缝，可以考虑克隆一份追加，然后滚到中点再重置）
        hsb->setValue(0);
    }
    else {
        hsb->setValue(next);
    }
}

/* =========================================================
 * 公告示例数据
 * ========================================================= */
void HomePage::addAnnouncement(const QString& text)
{
    if (ui->list_announcements)
        ui->list_announcements->addItem(text);
}

void HomePage::addSampleData()
{
    addProductCard("香氛淡香", "￥128", QPixmap(":/images/picturewall/11.jpg"));
    addProductCard("丝巾礼盒", "￥199", QPixmap(":/images/picturewall/12.jpg"));
    addProductCard("陶瓷杯组", "￥129", QPixmap(":/images/picturewall/13.jpg"));
    addProductCard("轻奢包包", "￥2090", QPixmap(":/images/picturewall/14.jpg"));
    addProductCard("柔光台灯", "￥159", QPixmap(":/images/picturewall/15.jpg"));

    addAnnouncement("春季上新满 300 减 50 活动中");
    addAnnouncement("周三会员日双倍积分");
    addAnnouncement("新增数字钱包支付方式");
}

/* =========================================================
 * 辅助：宽度比例（轮播加宽）
 * ========================================================= */
void HomePage::adjustWidths()
{
    int w = width();
    int leftW = int(w * kCarouselLeftRatio);
    if (leftW < kCarouselMinWidth) leftW = kCarouselMinWidth;

    ui->carouselFrame->setMinimumWidth(leftW);
    ui->carouselFrame->setMaximumWidth(leftW); // 固定宽度（右边占剩余）
}

/* =========================================================
 * 事件过滤：
 *  - 轮播悬停暂停/恢复
 *  - 卡片 hover 放大动画
 *  - 推荐区滚轮横向滚动 + 悬停暂停自动滚动
 * ========================================================= */
bool HomePage::eventFilter(QObject* obj, QEvent* event)
{
    // 轮播暂停/恢复
    if (obj == ui->carouselFrame) {
        if (event->type() == QEvent::Enter) {
            stopCarouselAutoPlay();
        }
        else if (event->type() == QEvent::Leave) {
            startCarouselAutoPlay();
        }
    }

    // 卡片 Hover 动画
    if (event->type() == QEvent::Enter || event->type() == QEvent::Leave) {
        if (auto* card = qobject_cast<QFrame*>(obj)) {
            if (card->property("class") == "productCard") {
                QRect g = card->geometry();
                QRect target = (event->type() == QEvent::Enter)
                    ? g.adjusted(-2, -2, 2, 2)
                    : g.adjusted(2, 2, -2, -2);

                auto* anim = new QPropertyAnimation(card, "geometry", card);
                anim->setDuration(160);
                anim->setEasingCurve(QEasingCurve::OutCubic);
                anim->setStartValue(g);
                anim->setEndValue(target);
                anim->start(QAbstractAnimation::DeleteWhenStopped);
            }
        }
    }

    // 精选推荐视口：悬停暂停自动滚动 + 鼠标滚轮横向滚动
    if (obj == ui->scroll_recommend->viewport()) {
        switch (event->type()) {
        case QEvent::Enter:
            m_autoScrollPaused = true;
            break;
        case QEvent::Leave:
            m_autoScrollPaused = false;
            break;
        case QEvent::Wheel: {
            auto* we = static_cast<QWheelEvent*>(event);
            #if QT_VERSION >= QT_VERSION_CHECK(6,0,0)
            int delta = we->angleDelta().y();
            #else
            int delta = we->delta();
            #endif
            if (auto* hsb = ui->scroll_recommend->horizontalScrollBar()) {
                hsb->setValue(hsb->value() - delta);
                return true; // 拦截垂直滚轮 → 用于横向
            }
            break;
        }
        default: break;
        }
    }

    return QWidget::eventFilter(obj, event);
}

/* =========================================================
 * 尺寸变化：同步轮播缩放 + 推荐宽/高调整
 * ========================================================= */
void HomePage::resizeEvent(QResizeEvent* e)
{
    QWidget::resizeEvent(e);
    adjustWidths();
    rescaleCarousel();
    updateRecommendWidth();
    adjustRecommendHeight();
}