#ifndef TEXTTICKER_H
#define TEXTTICKER_H

#include <QLabel>
#include <QTimer>

class TextTicker : public QLabel
{
    Q_OBJECT
public:
    explicit TextTicker(QWidget *parent = nullptr);
    ~TextTicker() override;

    // 设置显示文本（会自动重新测量与重置滚动）
    void setShowText(const QString &str);

    // 设置滚动速度（像素/步）
    void setSpeed(int pxPerStep);
    // 设置定时器间隔（毫秒）
    void setInterval(int ms);
    // 设置文本之间的间隔（循环空白）
    void setGap(int px);

protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void changeEvent(QEvent *event) override; // 处理字体变化时重新测量

private slots:
    void advance(); // 滚动推进

private:
    void recalc();  // 重新计算文本宽度
    void updateTimerState();

private:
    QString m_showText;
    int     m_offset;      // 当前滚动偏移（像素）
    int     m_speed;       // 每次推进的像素
    int     m_gap;         // 两次文本之间的空白
    int     m_textWidth;   // 文本像素宽度
    QTimer  m_timer;
};

#endif // TEXTTICKER_H
