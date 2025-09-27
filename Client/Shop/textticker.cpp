#include "textticker.h"
#include <QPainter>
#include <QFontMetrics>
#include <QResizeEvent>
#include <QEvent>

TextTicker::TextTicker(QWidget *parent)
    : QLabel(parent),
      m_offset(0),
      m_speed(2),
      m_gap(40),
      m_textWidth(0)
{
    setMinimumWidth(120);
    setMinimumHeight(24);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    // 默认文本（可为空）
    m_showText = QStringLiteral("TextTicker");

    connect(&m_timer, &QTimer::timeout, this, &TextTicker::advance);
    m_timer.start(30); // 默认 30ms 刷新
    recalc();
    updateTimerState();
}

TextTicker::~TextTicker() = default;

void TextTicker::setShowText(const QString &str)
{
    if (m_showText == str)
        return;
    m_showText = str;
    m_offset = 0;
    recalc();
    updateTimerState();
    update();
}

void TextTicker::setSpeed(int pxPerStep)
{
    m_speed = qMax(1, pxPerStep);
}

void TextTicker::setInterval(int ms)
{
    m_timer.setInterval(qMax(5, ms));
}

void TextTicker::setGap(int px)
{
    m_gap = qMax(10, px);
    recalc();
    update();
}

void TextTicker::recalc()
{
    QFontMetrics fm(font());
    m_textWidth = fm.horizontalAdvance(m_showText);
}

void TextTicker::updateTimerState()
{
    // 如果文本不需要滚动，则直接停在起点
    if (m_textWidth <= width()) {
        m_offset = 0;
        // 仍然保持定时器可运行（也可以停止），这里出于简单保留
    }
}

void TextTicker::advance()
{
    if (m_textWidth > width()) {
        m_offset += m_speed;
        int cycle = m_textWidth + m_gap;
        if (m_offset >= cycle)
            m_offset -= cycle;
        update();
    }
}

void TextTicker::paintEvent(QPaintEvent *event)
{
    QLabel::paintEvent(event);

        QPainter p(this);
    p.setRenderHint(QPainter::TextAntialiasing, true);
    QFontMetrics fm(font());

    int baseY = (height() + fm.ascent() - fm.descent()) / 2;

    if (m_textWidth <= width()) {
        // 不需要滚动，正常绘制（可左对齐或居中，这里左对齐）
        p.drawText(0, baseY, m_showText);
        return;
    }

    // 第一个文本起点
    int x1 = -m_offset;
    // 第二个文本起点（循环衔接）
    int x2 = x1 + m_textWidth + m_gap;

    p.drawText(x1, baseY, m_showText);
    p.drawText(x2, baseY, m_showText);
}

void TextTicker::resizeEvent(QResizeEvent *event)
{
    QLabel::resizeEvent(event);
    updateTimerState();
}

void TextTicker::changeEvent(QEvent *event)
{
    QLabel::changeEvent(event);
    // 如果字体改变，重新测量
    if (event->type() == QEvent::FontChange) {
        recalc();
        updateTimerState();
    }
}
