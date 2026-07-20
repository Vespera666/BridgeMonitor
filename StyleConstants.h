#ifndef STYLECONSTANTS_H
#define STYLECONSTANTS_H

#include <QColor>
#include <QStyle>
#include <QVector>
#include <QWidget>

namespace StyleConstants {

// 图表系列颜色（HistoryDataDialog 和 RealTimeDataDialog 共用）
inline QVector<QColor> chartColors()
{
    return {
        QColor("#e74c3c"),  // red
        QColor("#2980b9"),  // blue
        QColor("#27ae60"),  // green
        QColor("#8e44ad"),  // purple
        QColor("#e67e22"),  // orange
        QColor("#1abc9c"),  // teal
    };
}

// CSS class 属性名
inline constexpr const char *kCssProp = "cssClass";
inline constexpr const char *kCssInfo = "info";
inline constexpr const char *kCssError = "error";
inline constexpr const char *kCssSuccess = "success";
inline constexpr const char *kCssWarning = "warning";

// 统一布局常量 (px)
inline constexpr int kMarginManage = 12;
inline constexpr int kMarginEdit = 24;
inline constexpr int kMarginData = 10;
inline constexpr int kSpacingManage = 10;
inline constexpr int kSpacingData = 8;
inline constexpr int kFormSpacing = 12;
inline constexpr int kGroupBoxMaxH = 85;

// 辅助函数：设置 CSS class 并强制刷新
template<typename T>
inline void applyCssClass(T *widget, const char *className)
{
    widget->setProperty(kCssProp, className);
    widget->style()->unpolish(widget);
    widget->style()->polish(widget);
}

} // namespace StyleConstants

#endif // STYLECONSTANTS_H
