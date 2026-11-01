// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef SEAMLY3D_ATELIER_H
#define SEAMLY3D_ATELIER_H
#include <QIconEngine>
#include <QPainter>
#include <QPainterPath>
#include <QGuiApplication>
#include <QStyleHints>
#include <QPalette>

namespace Seamly3D::Atelier
{
inline bool dark()
{
#if QT_VERSION >= QT_VERSION_CHECK(6,5,0)
    const auto scheme=QGuiApplication::styleHints()->colorScheme();
    if (scheme!=Qt::ColorScheme::Unknown) return scheme==Qt::ColorScheme::Dark;
#endif
    return QGuiApplication::palette().color(QPalette::Window).lightness()<128;
}
// Authored vector icons: one 24-unit grid, two-unit round stroke, no bitmap artwork.
class StrokeIcon : public QIconEngine
{
    QString m_name;
public:
    explicit StrokeIcon(const QString &name) : m_name(name) {}
    QIconEngine *clone() const override { return new StrokeIcon(m_name); }
    void paint(QPainter *p,const QRect &rect,QIcon::Mode mode,QIcon::State) override
    {
        p->save(); p->setRenderHint(QPainter::Antialiasing);
        const auto side=qMin(rect.width(),rect.height());
        p->translate(rect.center().x()-side/2.0,rect.center().y()-side/2.0); p->scale(side/24.0,side/24.0);
        QColor ink=dark()?QColor("#e6e5dc"):QColor("#29483d"); if (mode==QIcon::Disabled) ink.setAlpha(90);
        p->setPen(QPen(ink,1.8,Qt::SolidLine,Qt::RoundCap,Qt::RoundJoin)); p->setBrush(Qt::NoBrush);
        auto line=[p](qreal x,qreal y,qreal a,qreal b) { p->drawLine(QPointF(x,y),QPointF(a,b)); };
        if (m_name=="open") { QPainterPath path; path.moveTo(3,19); path.lineTo(3,6); path.lineTo(9,6); path.lineTo(11,9); path.lineTo(21,9); path.lineTo(18,19); path.closeSubpath(); p->drawPath(path); line(3,19,6,12); line(6,12,16,12); }
        else if (m_name=="save") { line(12,3,12,15); line(8,11,12,15); line(16,11,12,15); QPainterPath path; path.moveTo(4,15); path.lineTo(4,20); path.lineTo(20,20); path.lineTo(20,15); p->drawPath(path); }
        else if (m_name=="fit") { for (int i=0;i<4;++i) { p->save(); p->translate(12,12); p->rotate(i*90); line(-8,-3,-8,-8); line(-8,-8,-3,-8); p->restore(); } }
        else if (m_name=="mesh") { QPolygonF shape; shape<<QPointF(12,3)<<QPointF(21,12)<<QPointF(12,21)<<QPointF(3,12)<<QPointF(12,3); p->drawPolyline(shape); line(3,12,21,12); line(12,3,12,21); }
        else if (m_name=="undo" || m_name=="redo") {
            if (m_name=="redo") { p->translate(24,0); p->scale(-1,1); }
            QPainterPath path; path.moveTo(5,8); path.lineTo(14,8); path.cubicTo(22,8,22,19,14,19); p->drawPath(path);
            line(5,8,10,3); line(5,8,10,13);
        }
        else if (m_name=="reset" || m_name=="rotate") { p->drawArc(QRectF(4,4,16,16),40*16,280*16); line(18,3,20,8); line(20,8,15,8); }
        else if (m_name=="move") { line(12,3,12,21); line(3,12,21,12); line(12,3,9,6); line(12,3,15,6); line(21,12,18,9); line(21,12,18,15); line(12,21,9,18); line(12,21,15,18); line(3,12,6,9); line(3,12,6,15); }
        else if (m_name=="select") { QPolygonF shape; shape<<QPointF(5,3)<<QPointF(19,13)<<QPointF(12,14)<<QPointF(9,21)<<QPointF(5,3); p->drawPolyline(shape); }
        else if (m_name=="form") { QPainterPath path; path.moveTo(9,3); path.lineTo(15,3); path.lineTo(15,6); path.lineTo(20,9); path.lineTo(16,14); path.lineTo(18,21); path.lineTo(6,21); path.lineTo(8,14); path.lineTo(4,9); path.lineTo(9,6); path.closeSubpath(); p->drawPath(path); }
        else if (m_name=="pieces") { p->drawRoundedRect(QRectF(4,7,12,14),1,1); QPainterPath path; path.moveTo(9,3); path.lineTo(20,3); path.lineTo(20,17); p->drawPath(path); }
        else if (m_name=="seam") { line(4,20,20,4); p->drawEllipse(QRectF(15,3,6,6)); line(4,8,8,12); line(8,4,12,8); line(12,16,16,20); line(16,12,20,16); }
        else if (m_name=="play") { QPolygonF shape; shape<<QPointF(8,4)<<QPointF(20,12)<<QPointF(8,20)<<QPointF(8,4); p->drawPolyline(shape); }
        else if (m_name=="pause") { line(8,5,8,19); line(16,5,16,19); }
        p->restore();
    }
    QPixmap pixmap(const QSize &size,QIcon::Mode mode,QIcon::State state) override
    {
        QPixmap result(size); result.fill(Qt::transparent); QPainter p(&result); paint(&p,QRect(QPoint(),size),mode,state); return result;
    }
};
inline QIcon icon(const QString &name) { return QIcon(new StrokeIcon(name)); }
inline QPalette palette()
{
    QPalette result=QGuiApplication::palette(); const bool d=dark();
    result.setColor(QPalette::Window,QColor(d?"#202a26":"#f1eee6"));
    result.setColor(QPalette::Base,QColor(d?"#27332d":"#fcfbf7"));
    result.setColor(QPalette::AlternateBase,QColor(d?"#303c34":"#e8e5dc"));
    result.setColor(QPalette::Button,QColor(d?"#303d35":"#e7e7dd"));
    result.setColor(QPalette::WindowText,QColor(d?"#eeeade":"#243b32"));
    result.setColor(QPalette::Text,result.color(QPalette::WindowText));
    result.setColor(QPalette::ButtonText,result.color(QPalette::WindowText));
    result.setColor(QPalette::Highlight,QColor(d?"#accbb2":"#2c604e"));
    result.setColor(QPalette::HighlightedText,QColor(d?"#182c22":"#ffffff"));
    result.setColor(QPalette::Mid,QColor(d?"#768579":"#8e9585"));
    result.setColor(QPalette::Disabled,QPalette::Text,QColor(d?"#879388":"#787f73"));
    result.setColor(QPalette::Disabled,QPalette::ButtonText,result.color(QPalette::Disabled,QPalette::Text));
    return result;
}
inline QString styleSheet()
{
    auto sheet=QStringLiteral(R"(
QWidget { color: palette(window-text); }
QMainWindow#seamlyWorkspace { background: palette(window); color: palette(window-text); }
QToolBar { border: none; spacing: 6px; padding: 7px 14px; background: palette(window); }
QToolBar#sceneToolbar { border-bottom: 1px solid palette(mid); padding: 4px 12px; spacing: 4px; }
QLabel#workspaceBrand { font-size: 18px; font-weight: 600; padding-right: 20px; }
QToolButton { border: 1px solid transparent; border-radius: 5px; padding: 4px 8px; color: palette(button-text); }
QToolButton:hover { background: palette(button); }
QToolButton:checked { background: palette(button); border-color: palette(mid); }
QToolButton:focus, QPushButton:focus { border: 1px solid palette(highlight); }
QPushButton { background: palette(button); color: palette(button-text); border: 1px solid palette(mid); border-radius: 5px; padding: 7px 12px; }
QPushButton:hover { background: palette(alternate-base); }
QPushButton:pressed { border-color: palette(highlight); }
QPushButton:disabled { color: palette(disabled, button-text); border-color: palette(button); }
QPushButton#assembleButton { background: palette(highlight); color: palette(highlighted-text); border-color: palette(highlight); font-weight: 600; padding: 7px 18px; }
QPushButton#assembleButton:disabled { background: palette(button); color: palette(disabled, button-text); border-color: palette(button); }
QTabWidget::pane { border: 0; background: palette(window); }
QTabBar::tab { color: palette(window-text); padding: 12px 10px; border: none; border-bottom: 2px solid transparent; }
QTabBar::tab:selected { background: palette(base); border-bottom-color: palette(highlight); }
QTabBar::tab:hover { background: palette(button); }
QScrollArea, QScrollArea > QWidget > QWidget { background: palette(window); }
QLabel[sectionHeading="true"] { font-size: 14px; font-weight: 600; padding-top: 8px; }
QLabel[contextNote="true"] { color: palette(window-text); }
QComboBox, QSpinBox, QDoubleSpinBox, QLineEdit { color: palette(text); background: palette(base); border: 1px solid palette(mid); border-radius: 4px; padding: 5px 6px; min-height: 18px; }
QComboBox:focus, QSpinBox:focus, QDoubleSpinBox:focus, QLineEdit:focus { border-color: palette(highlight); }
QComboBox:disabled, QSpinBox:disabled, QDoubleSpinBox:disabled, QLineEdit:disabled { color: palette(disabled, text); background: palette(window); }
QListWidget { background: palette(base); color: palette(text); border: 1px solid palette(mid); padding: 4px; }
QListWidget::item { padding: 5px 2px; }
QListWidget::item:selected { background: palette(highlight); color: palette(highlighted-text); }
QStatusBar { background: palette(window); border-top: 1px solid palette(mid); padding: 3px 10px; }
QStatusBar::item { border: none; }
QSplitter::handle { background: palette(mid); }
QWidget#seamEditorFooter { border-top: 1px solid palette(mid); }
QScrollBar:vertical { background: transparent; width: 10px; margin: 2px; }
QScrollBar::handle:vertical { background: palette(mid); min-height: 28px; border-radius: 3px; }
QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }
QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical { background: transparent; }
QSpinBox::up-button, QDoubleSpinBox::up-button { subcontrol-origin: border; subcontrol-position: top right; width: 18px; }
QSpinBox::down-button, QDoubleSpinBox::down-button { subcontrol-origin: border; subcontrol-position: bottom right; width: 18px; }
)");
    // Resolve stylesheet colours explicitly: Qt's stylesheet palette() can retain
    // the previous native appearance when macOS switches light/dark at runtime.
    const auto p=palette();
    sheet.replace(QStringLiteral("palette(disabled, button-text)"),p.color(QPalette::Disabled,QPalette::ButtonText).name());
    sheet.replace(QStringLiteral("palette(disabled, text)"),p.color(QPalette::Disabled,QPalette::Text).name());
    const QPair<const char *,QPalette::ColorRole> roles[]={
        {"window",QPalette::Window},{"window-text",QPalette::WindowText},{"base",QPalette::Base},
        {"alternate-base",QPalette::AlternateBase},{"button",QPalette::Button},{"button-text",QPalette::ButtonText},
        {"text",QPalette::Text},{"mid",QPalette::Mid},{"highlight",QPalette::Highlight},{"highlighted-text",QPalette::HighlightedText}};
    for (const auto &role : roles)
        sheet.replace(QStringLiteral("palette(%1)").arg(QString::fromLatin1(role.first)),p.color(role.second).name());
    return sheet;
}
}
#endif
