#ifndef HOMEPAGE_H
#define HOMEPAGE_H

#include <QWidget>
#include <QVector>
#include <QPixmap>

class ShoppingClient;
class QLabel;
class QTimer;

namespace Ui {
    class HomePage;
}

/*
 * HomePage
 *  - 左侧轮播图（自动播放 / 指示点 / 悬停暂停）
 *  - 右侧“精选推荐”横向滚动（可选自动滚动 / 手动滚动）
 *  - 公告列表
 *
 * 代码结构核心分区：
 *  1. 生命周期：构造 / 析构 / setClient / loadInitialData
 *  2. 轮播：数据结构 + 添加 + 缩放 + 控制
 *  3. 精选推荐：卡片生成 + 容器宽度计算 + 高度自适应 + 自动滚动
 *  4. 公告
 *  5. 事件过滤（悬停暂停、卡片 hover 动画、滚轮横向滚动等）
 */

class HomePage : public QWidget
{
    Q_OBJECT
public:
    explicit HomePage(QWidget* parent = nullptr);
    ~HomePage();

    void setClient(ShoppingClient* client);
    void loadInitialData();

    // 外部（将来）如果需要动态增加内容：
    void addAnnouncement(const QString& text);
    void addProductCard(const QString& name,
        const QString& price,
        const QPixmap& pix,
        const QString& subTitle = QString());

protected:
    bool eventFilter(QObject* obj, QEvent* event) override;
    void resizeEvent(QResizeEvent* e) override;

private:
    Ui::HomePage* ui = nullptr;
    ShoppingClient* m_client = nullptr;

    /* ===================== 轮播相关 ===================== */
    struct CarouselSlide {
        QLabel* label = nullptr;  // 显示载体
        QPixmap original;         // 原始图（确保窗口变化可重新等比缩放）
    };
    QVector<CarouselSlide> m_slides;

    QTimer* m_carouselTimer = nullptr;
    int     m_carouselIntervalMs = 3000;  // 轮播切换间隔（毫秒）

    QVector<QLabel*> m_indicatorDots;     // 指示点标签

    void initUi();
    void initSignals();
    void initCarousel();

    void addCarouselSlide(const QString& sourcePath);
    void clearCarousel();
    void rescaleCarousel();                // 窗口变化后重新缩放所有轮播图（等比）
    void rebuildDots();                    // 根据当前页数重建指示点
    void updateDots(int activeIndex);

    void startCarouselAutoPlay();
    void stopCarouselAutoPlay();

    /* ===================== 精选推荐 / 卡片 ===================== */
    QWidget* makeProductCardWidget(const QString& name,
        const QString& price,
        const QPixmap& pix,
        const QString& subTitle);

    void updateRecommendWidth();   // 横向宽度（决定是否滚动）
    void adjustRecommendHeight();  // 包含标题 + 卡片的整体高度
    void adjustWidths();           // 中间主布局左右宽度比例（轮播加宽）

    // 自动横向滚动
    QTimer* m_recommendAutoTimer = nullptr;
    bool    m_autoScrollPaused = false;
    void startAutoScrollRecommend();
    void stopAutoScrollRecommend();
    void tickAutoScrollRecommend();

    /* ===================== 示例数据 ===================== */
    void addSampleData();
};

#endif // HOMEPAGE_H