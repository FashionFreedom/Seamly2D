/******************************************************************************
 *   @file   color_combobox.cpp                                               *
 *****************************************************************************/
#include <QColor>
#include <QPixmap>
#include <QMap>
#include <QAbstractItemView>
#include <QComboBox>

#include "../vtools/tools/vabstracttool.h"
#include "../vmisc/logging.h"
#include "../ifc/ifcdef.h"

#include "color_combobox.h"

Q_LOGGING_CATEGORY(colorComboBox, "color_combobox")

/*
 * Default Constructor.
 */
ColorComboBox::ColorComboBox(QWidget *parent, const char *name)
    : QComboBox(parent)
    , m_currentColor("black")
    , m_iconWidth(40)
    , m_iconHeight(14)
{
    setObjectName(name);
    setEditable (false);
    init();
}

/*
 * Constructor that provides width and height for icon.
 */
ColorComboBox::ColorComboBox(int width, int height, QWidget *parent, const char *name)
    : QComboBox(parent)
    , m_currentColor("black")
    , m_iconWidth(width)
    , m_iconHeight(height)
{
    qCDebug(colorComboBox, "ColorComboBox Constructor 2 used");
    setObjectName(name);
    setEditable (false);
    init();
}

/**
 * Destructor
 */
ColorComboBox::~ColorComboBox(){}

/*
 * Initialisation called from constructor or manually but only once.
 */
void ColorComboBox::init()
{
    setItems(VAbstractTool::ColorsList());
    connect(this, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ColorComboBox::colorChanged);
}

/*
 * Sets the items shown in the combobox and sets index to the 1st color.
 */
void ColorComboBox::setItems(QMap<QString, QString> map)
{
    this->blockSignals(true);

    clear();

#if defined(Q_OS_MAC)
    // Mac pixmap should be little bit smaller
    setIconSize(QSize(m_iconWidth-= 2 ,m_iconHeight-= 2));
#else
    // Windows
    setIconSize(QSize(m_iconWidth, m_iconHeight));
#endif

    this->view()->setTextElideMode(Qt::ElideNone);
    map.remove(ColorByGroup);
    QMap<QString, QString>::const_iterator i = map.constBegin();
    while (i != map.constEnd())
    {
        QPixmap pixmap = VAbstractTool::createColorIcon(m_iconWidth, m_iconHeight, i.key());
        addItem(QIcon(pixmap), i.value(), QVariant(i.key()));
        ++i;
    }

    setMaxVisibleItems(map.size());
    this->model()->sort(1, Qt::AscendingOrder);
    setCurrentIndex(0);
    colorChanged(currentIndex());

    this->blockSignals(false);
}
/*
 * Sets the color shown in the combobox to the given color.
 */
void ColorComboBox::setColor(const QString &color)
{
    qCDebug(colorComboBox, "ColorComboBox::setColor");
    m_currentColor = color;

    setCurrentIndex(findData(color));

    if (currentIndex()!= count() -1 )
    {
        colorChanged(currentIndex());
    }
}

/*
 * Called when the color has changed. This method sets the current color to the
 * value chosen.
 */
void ColorComboBox::colorChanged(int index)
{
    qCDebug(colorComboBox, "ColorComboBox::colorChanged");

    QVariant color = itemData(index);
    if(color != QVariant::Invalid )
    {
       m_currentColor = QVariant(color).toString();
    }

    emit colorChangedSignal(m_currentColor);
}

QString ColorComboBox::getColor() const
{
    return m_currentColor;
}

int ColorComboBox::getIconWidth()
{
    return m_iconWidth;
}

int ColorComboBox::getIconHeight()
{
    return m_iconHeight;
}
