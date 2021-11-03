/******************************************************************************
 *   @file   lineweight_combobox.cpp                                         *
 *****************************************************************************/
#include <QAbstractItemView>

#include "../vmisc/logging.h"

#include "dc_lineweightcombobox.h"

Q_LOGGING_CATEGORY(dcLineWeightComboBox, "dc_linewidthcombobox")

/**
 * Constructor with name.
 */
LineWeightComboBox::LineWeightComboBox(QWidget *parent, const char *name)
		: QComboBox(parent)
      	, m_currentWeight(DC2::Weight00)
		, m_iconWidth(40)
		, m_iconHeight(14)
{
		 setObjectName(name);
		 setEditable ( false );
		 init();
}

/**
 * Constructor that provides a width and height for the icon.
 */
LineWeightComboBox::LineWeightComboBox(int width, int height, QWidget *parent, const char *name)
		: QComboBox(parent)
		, m_currentWeight(DC2::Weight00)
		, m_iconWidth(width)
		, m_iconHeight(height)
{
		 setObjectName(name);
		 setEditable ( false );
		 init();
}

/**
 * Destructor
 */
LineWeightComboBox::~LineWeightComboBox() {}

/**
 * Initialisation called from constructor or manually but only once.
 */
void LineWeightComboBox::init()
{
		qCDebug(dcLineWeightComboBox, "LineWeightComboBox::init");
    this->blockSignals(true);

#if defined(Q_OS_MAC)
    setIconSize(QSize(m_iconWidth-= 2 ,m_iconHeight-= 2)); // On Mac pixmap should be little bit smaller.
#else // Windows
    setIconSize(QSize(m_iconWidth, m_iconHeight));
#endif

    this->view()->setTextElideMode(Qt::ElideNone);

    addItem(QIcon(":/icon/40x12/lineweight_01.png"), tr("0.00mm"));
    addItem(QIcon(":/icon/40x12/lineweight_01.png"), tr("0.05mm"));
    addItem(QIcon(":/icon/40x12/lineweight_01.png"), tr("0.09mm"));
    addItem(QIcon(":/icon/40x12/lineweight_01.png"), tr("0.13mm (ISO)"));
    addItem(QIcon(":/icon/40x12/lineweight_01.png"), tr("0.15mm"));
    addItem(QIcon(":/icon/40x12/lineweight_01.png"), tr("0.18mm (ISO)"));
    addItem(QIcon(":/icon/40x12/lineweight_02.png"), tr("0.20mm"));
    addItem(QIcon(":/icon/40x12/lineweight_02.png"), tr("0.25mm (ISO)"));
    addItem(QIcon(":/icon/40x12/lineweight_03.png"), tr("0.30mm"));
    addItem(QIcon(":/icon/40x12/lineweight_03.png"), tr("0.35mm (ISO)"));
    addItem(QIcon(":/icon/40x12/lineweight_04.png"), tr("0.40mm"));
    addItem(QIcon(":/icon/40x12/lineweight_04.png"), tr("0.50mm (ISO)"));
    addItem(QIcon(":/icon/40x12/lineweight_05.png"), tr("0.53mm"));
    addItem(QIcon(":/icon/40x12/lineweight_05.png"), tr("0.60mm"));
    addItem(QIcon(":/icon/40x12/lineweight_06.png"), tr("0.70mm (ISO)"));
    addItem(QIcon(":/icon/40x12/lineweight_07.png"), tr("0.80mm"));
    addItem(QIcon(":/icon/40x12/lineweight_08.png"), tr("0.90mm"));
    addItem(QIcon(":/icon/40x12/lineweight_09.png"), tr("1.00mm (ISO)"));
    addItem(QIcon(":/icon/40x12/lineweight_10.png"), tr("1.06mm"));
    addItem(QIcon(":/icon/40x12/lineweight_10.png"), tr("1.20mm"));
    addItem(QIcon(":/icon/40x12/lineweight_11.png"), tr("1.40mm (ISO)"));
    addItem(QIcon(":/icon/40x12/lineweight_11.png"), tr("1.58mm"));
    addItem(QIcon(":/icon/40x12/lineweight_12.png"), tr("2.00mm (ISO)"));
    addItem(QIcon(":/icon/40x12/lineweight_12.png"), tr("2.11mm"));

    this->blockSignals(false);
		connect(this, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &LineWeightComboBox::lineWeightChangedSlot);

    setCurrentIndex(0);
    lineWeightChangedSlot(currentIndex());
}

DC2::LineWeight LineWeightComboBox::getLineWeight() const
{
    return m_currentWeight;
}

/**
 * Sets the currently selected weight item to the given weight.
 */
void LineWeightComboBox::setLineWeight(DC2::LineWeight weight)
{
     qCDebug(dcLineWeightComboBox, "LineWeightComboBox::setLineWeight %d", (int) weight);

     switch (weight)
		 {
     		 case DC2::WeightDefault:
         		setCurrentIndex(0);
        		break;
    		 case DC2::Weight00:
        		setCurrentIndex(1);
        		break;
    		case DC2::Weight01:
        		setCurrentIndex(2);
        		break;
        case DC2::Weight02:
        		setCurrentIndex(3);
        		break;
    		case DC2::Weight03:
        		setCurrentIndex(4);
        		break;
    	  case DC2::Weight04:
        		setCurrentIndex(5);
        		break;
    		case DC2::Weight05:
        		setCurrentIndex(6);
        		break;
    		case DC2::Weight06:
        		setCurrentIndex(7);
        		break;
    		case DC2::Weight07:
        		setCurrentIndex(8);
        		break;
    		case DC2::Weight08:
        		setCurrentIndex(9);
        		break;
    		case DC2::Weight09:
        		setCurrentIndex(10);
        		break;
    		case DC2::Weight10:
        		setCurrentIndex(11);
        		break;
    		case DC2::Weight11:
        		setCurrentIndex(12);
        		break;
    		case DC2::Weight12:
        		setCurrentIndex(13);
        		break;
    		case DC2::Weight13:
        		setCurrentIndex(14);
        		break;
    		case DC2::Weight14:
        		setCurrentIndex(15);
        		break;
    		case DC2::Weight15:
        		setCurrentIndex(16);
        		break;
    		case DC2::Weight16:
        		setCurrentIndex(17);
        		break;
    		case DC2::Weight17:
        		setCurrentIndex(18);
        		break;
    		case DC2::Weight18:
        		setCurrentIndex(19);
        		break;
    		case DC2::Weight19:
        		setCurrentIndex(20);
        		break;
    		case DC2::Weight20:
        		setCurrentIndex(21);
        		break;
    		case DC2::Weight21:
        		setCurrentIndex(22);
        		break;
    		case DC2::Weight22:
        		setCurrentIndex(23);
        		break;
    		case DC2::Weight23:
        		setCurrentIndex(24);
        		break;
    		default:
        		break;
    }

    lineWeightChangedSlot(currentIndex());
}

/**
 * Called when the width has changed. This method sets the current width to the value chosen or even
 * offers a dialog to the user that allows him/ her to choose an individual width.
 */
void LineWeightComboBox::lineWeightChangedSlot(int index)
{
    qCDebug(dcLineWeightComboBox, "LineWeightComboBox::lineWeightChangedSlot %d", (int)index);

  	switch (index)
		{
      	case 0:
          	m_currentWeight = DC2::WeightDefault;
          	break;
      	case 1:
          	m_currentWeight = DC2::Weight00;
          	break;
      	case 2:
          	m_currentWeight = DC2::Weight01;
          	break;
      	case 3:
          	m_currentWeight = DC2::Weight02;
          	break;
      	case 4:
          	m_currentWeight = DC2::Weight03;
          	break;
      	case 5:
          	m_currentWeight = DC2::Weight04;
          	break;
      	case 6:
          	m_currentWeight = DC2::Weight05;
          	break;
      	case 7:
          	m_currentWeight = DC2::Weight06;
          	break;
      	case 8:
          	m_currentWeight = DC2::Weight07;
          	break;
      	case 9:
          	m_currentWeight = DC2::Weight08;
          	break;
      	case 10:
          	m_currentWeight = DC2::Weight09;
          	break;
      	case 11:
          	m_currentWeight = DC2::Weight10;
          	break;
      	case 12:
          	m_currentWeight = DC2::Weight11;
          	break;
      	case 13:
          	m_currentWeight = DC2::Weight12;
          	break;
      	case 14:
          	m_currentWeight = DC2::Weight13;
          	break;
      	case 15:
          	m_currentWeight = DC2::Weight14;
          	break;
      	case 16:
          	m_currentWeight = DC2::Weight15;
          	break;
      	case 17:
          	m_currentWeight = DC2::Weight16;
          	break;
      	case 18:
          	m_currentWeight = DC2::Weight17;
          	break;
      	case 19:
          	m_currentWeight = DC2::Weight18;
          	break;
      	case 20:
          	m_currentWeight = DC2::Weight19;
          	break;
      	case 21:
          	m_currentWeight = DC2::Weight20;
          	break;
      	case 22:
          	m_currentWeight = DC2::Weight21;
          	break;
      	case 23:
          	m_currentWeight = DC2::Weight22;
	          break;
      	case 24:
          	m_currentWeight = DC2::Weight23;
          	break;
      	default:
          	break;
    }

	  qCDebug(dcLineWeightComboBox, "Current line weight is (%d): %d\n",(int)index, ((int)m_currentWeight));

    emit lineWeightChangedSignal(m_currentWeight);
}
