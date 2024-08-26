/******************************************************************************
 *   @file   tmainwindow.cpp
 **  @author Douglas S Caskey
 **  @date   25 Jan, 2024
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Seamly2D project, a pattern making
 **  program to create and model patterns of clothing.
 **  Copyright (C) 2017-2024 Seamly2D project
 **  <https://github.com/fashionfreedom/seamly2d> All Rights Reserved.
 **
 **  Seamly2D is free software: you can redistribute it and/or modify
 **  it under the terms of the GNU General Public License as published by
 **  the Free Software Foundation, either version 3 of the License, or
 **  (at your option) any later version.
 **
 **  Seamly2D is distributed in the hope that it will be useful,
 **  but WITHOUT ANY WARRANTY; without even the implied warranty of
 **  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 **  GNU General Public License for more details.
 **
 **  You should have received a copy of the GNU General Public License
 **  along with Seamly2D.  If not, see <http://www.gnu.org/licenses/>.
 **
 *************************************************************************/

 /************************************************************************
 **
 **  @file   tmainwindow.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   10 7, 2015
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2015 Valentina project
 **  <https://bitbucket.org/dismine/valentina> All Rights Reserved.
 **
 **  Valentina is free software: you can redistribute it and/or modify
 **  it under the terms of the GNU General Public License as published by
 **  the Free Software Foundation, either version 3 of the License, or
 **  (at your option) any later version.
 **
 **  Valentina is distributed in the hope that it will be useful,
 **  but WITHOUT ANY WARRANTY; without even the implied warranty of
 **  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 **  GNU General Public License for more details.
 **
 **  You should have received a copy of the GNU General Public License
 **  along with Valentina.  If not, see <http://www.gnu.org/licenses/>.
 **
 *************************************************************************/

#include "tmainwindow.h"
#include "ui_tmainwindow.h"
#include "dialogs/dialogaboutseamlyme.h"
#include "dialogs/new_measurements_dialog.h"
#include "dialogs/dialogmdatabase.h"
#include "dialogs/dialogseamlymepreferences.h"
#include "dialogs/dialogexporttocsv.h"
#include "dialogs/me_shortcuts_dialog.h"
#include "../vpatterndb/calculator.h"
#include "../vpatterndb/pmsystems.h"
#include "../ifc/ifcdef.h"
#include "../ifc/xml/individual_size_converter.h"
#include "../ifc/xml/multi_size_converter.h"
#include "../ifc/xml/vpatternconverter.h"
#include "../vmisc/def.h"
#include "../vmisc/vlockguard.h"
#include "../vmisc/vsysexits.h"
#include "../vmisc/qxtcsvmodel.h"
#include "vlitepattern.h"
#include "../qmuparser/qmudef.h"
#include "../vtools/dialogs/support/edit_formula_dialog.h"
#include "version.h"
#include "../vformat/measurements.h"
#include "application_me.h" // Should be last because of definning qApp

#include <QClipboard>
#include <QComboBox>
#include <QDesktopServices>
#include <QFileDialog>
#include <QFileInfo>
#include <QLabel>
#include <QMessageBox>
#include <QPixmap>
#include <QPrinter>
#include <QPrintPreviewDialog>
#include <QProcess>
#include <QTextCodec>
#include <QtNumeric>

#if defined(Q_OS_MAC)
#include <QMimeData>
#include <QDrag>
#endif //defined(Q_OS_MAC)

#define DIALOG_MAX_FORMULA_HEIGHT 64

QT_WARNING_PUSH
QT_WARNING_DISABLE_CLANG("-Wmissing-prototypes")
QT_WARNING_DISABLE_INTEL(1418)

Q_LOGGING_CATEGORY(tMainWindow, "t.mainwindow")

QT_WARNING_POP

// We need this enum in case we will add or delete a column. And also make code more readable.
enum {ColumnName = 0, ColumnNumber, ColumnFullName, ColumnCalcValue, ColumnFormula, ColumnBaseValue, ColumnInSizes, ColumnInHeights};

//---------------------------------------------------------------------------------------------------------------------
TMainWindow::TMainWindow(QWidget *parent)
	: VAbstractMainWindow(parent),
	  ui(new Ui::TMainWindow),
	  individualMeasurements(nullptr),
	  data(nullptr),
	  mUnit(Unit::Cm),
	  pUnit(Unit::Cm),
	  mType(MeasurementsType::Individual),
	  currentSize(0),
	  currentHeight(0),
	  curFile(),
	  gradationHeights(nullptr),
	  gradationSizes(nullptr),
	  comboBoxUnits(nullptr),
	  lock(nullptr),
	  search(),
	  labelGradationHeights(nullptr),
	  labelGradationSizes(nullptr),
	  labelPatternUnit(nullptr),
	  actionDockDiagram(nullptr),
	  dockDiagramVisible(true),
	  isInitialized(false),
	  m_isReadOnly(false),
	  recentFileActs(),
	  separatorAct(nullptr),
	  hackedWidgets()
{
	ui->setupUi(this);

	qApp->Settings()->getOsSeparator() ? setLocale(QLocale()) : setLocale(QLocale::c());

	ui->lineEditFind->setClearButtonEnabled(true);
	ui->lineEditName->setClearButtonEnabled(true);
	ui->lineEditFullName->setClearButtonEnabled(true);
	ui->lineEditGivenName->setClearButtonEnabled(true);
	ui->lineEditFamilyName->setClearButtonEnabled(true);
	ui->lineEditEmail->setClearButtonEnabled(true);

	ui->lineEditFind->installEventFilter(this);
	ui->plainTextEditFormula->installEventFilter(this);

	search = QSharedPointer<VTableSearch>(new VTableSearch(ui->tableWidget));
	ui->tabWidget->setVisible(false);

	ui->mainToolBar->setContextMenuPolicy(Qt::PreventContextMenu);
	ui->toolBarGradation->setContextMenuPolicy(Qt::PreventContextMenu);

	//MSVC doesn't support int arrays in initializer list
	for (int i = 0; i < MaxRecentFiles; ++i)
	{
		recentFileActs[i] = nullptr;
	}

	SetupMenu();
	UpdateWindowTitle();
	ReadSettings();

#if defined(Q_OS_MAC)
	// On Mac default icon size is 32x32.
	ui->toolBarGradation->setIconSize(QSize(24, 24));

	ui->pushButtonShowInExplorer->setText(tr("Show in Finder"));

	// Mac OS Dock Menu
	QMenu *menu = new QMenu(this);
	connect(menu, &QMenu::aboutToShow, this, &TMainWindow::AboutToShowDockMenu);
	AboutToShowDockMenu();
	menu->setAsDockMenu();
#endif //defined(Q_OS_MAC)
}

//---------------------------------------------------------------------------------------------------------------------
TMainWindow::~TMainWindow()
{
	delete data;
	delete individualMeasurements;
	delete ui;
}

//---------------------------------------------------------------------------------------------------------------------
QString TMainWindow::CurrentFile() const
{
	return curFile;
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::RetranslateTable()
{
	if (individualMeasurements != nullptr)
	{
		const int row = ui->tableWidget->currentRow();
		RefreshTable();
		ui->tableWidget->selectRow(row);
		search->RefreshList(ui->lineEditFind->text());
	}
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::SetBaseMHeight(int height)
{
	if (individualMeasurements != nullptr)
	{
		if (mType == MeasurementsType::Multisize)
		{
			const int row = ui->tableWidget->currentRow();
			currentHeight = UnitConvertor(height, Unit::Cm, mUnit);
			RefreshData();
			ui->tableWidget->selectRow(row);
		}
	}
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::SetBaseMSize(int size)
{
	if (individualMeasurements != nullptr)
	{
		if (mType == MeasurementsType::Multisize)
		{
			const int row = ui->tableWidget->currentRow();
			currentSize = UnitConvertor(size, Unit::Cm, mUnit);
			RefreshData();
			ui->tableWidget->selectRow(row);
		}
	}
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::SetPUnit(Unit unit)
{
	pUnit = unit;
	UpdatePatternUnit();
}

//---------------------------------------------------------------------------------------------------------------------
bool TMainWindow::LoadFile(const QString &path)
{
    QString filename = path;
	if (individualMeasurements == nullptr)
	{
		if (!QFileInfo(filename).exists())
		{
			qCCritical(tMainWindow, "%s", qUtf8Printable(tr("File '%1' doesn't exist!").arg(filename)));
			if (qApp->isTestMode())
			{
				qApp->exit(V_EX_NOINPUT);
			}
			return false;
		}

		// Check if file already opened
		QList<TMainWindow*>list = qApp->mainWindows();
		for (int i = 0; i < list.size(); ++i)
		{
			if (list.at(i)->CurrentFile() == filename)
			{
				list.at(i)->activateWindow();
				close();
				return false;
			}
		}

		VlpCreateLock(lock, filename);

		if (!lock->IsLocked())
		{
			if (!IgnoreLocking(lock->GetLockError(), filename))
			{
				return false;
			}
		}

		try
		{
			data = new VContainer(qApp->translateVariables(), &mUnit);

			individualMeasurements = new MeasurementDoc(data);
			individualMeasurements->setSize(&currentSize);
			individualMeasurements->setHeight(&currentHeight);
			individualMeasurements->setXMLContent(filename);

			mType = individualMeasurements->Type();

			if (mType == MeasurementsType::Unknown)
			{
				VException e(tr("File has unknown format."));
				throw e;
			}

			if (mType == MeasurementsType::Multisize)
			{
				MultiSizeConverter converter(filename);
				m_curFileFormatVersion = converter.getCurrentFormatVersion();
				m_curFileFormatVersionStr = converter.getVersionStr();
				individualMeasurements->setXMLContent(converter.Convert());// Read again after conversion
                filename.replace(QLatin1String(".vst"), QLatin1String(".smms"));
			}
			else
			{
				IndividualSizeConverter converter(filename);
				m_curFileFormatVersion = converter.getCurrentFormatVersion();
				m_curFileFormatVersionStr = converter.getVersionStr();
				individualMeasurements->setXMLContent(converter.Convert());// Read again after conversion
                filename.replace(QLatin1String(".vit"), QLatin1String(".smis"));
			}

			if (!individualMeasurements->eachKnownNameIsValid())
			{
				VException e(tr("File contains invalid known measurement(s)."));
				throw e;
			}

			mUnit = individualMeasurements->measurementUnits();
			pUnit = mUnit;

			currentSize = individualMeasurements->BaseSize();
			currentHeight = individualMeasurements->BaseHeight();

			ui->labelToolTip->setVisible(false);
			ui->tabWidget->setVisible(true);

			m_isReadOnly = individualMeasurements->isReadOnly();
			UpdatePadlock(m_isReadOnly);

			SetCurrentFile(filename);

			InitWindow();

			const bool freshCall = true;
			RefreshData(freshCall);

			if (ui->tableWidget->rowCount() > 0)
			{
				ui->tableWidget->selectRow(0);
			}

			MeasurementGUI();
		}
		catch (VException &exception)
		{
			qCCritical(tMainWindow, "%s\n\n%s\n\n%s", qUtf8Printable(tr("File error.")),
					   qUtf8Printable(exception.ErrorMessage()), qUtf8Printable(exception.DetailedInformation()));

			ui->labelToolTip->setVisible(true);
			ui->tabWidget->setVisible(false);
			delete individualMeasurements;
			individualMeasurements = nullptr;
			delete data;
			data = nullptr;
			lock.reset();

			if (qApp->isTestMode())
			{
				qApp->exit(V_EX_NOINPUT);
			}
			return false;
		}
	}
	else
	{
		qApp->newMainWindow();
		return qApp->mainWindow()->LoadFile(filename);
	}

	return true;
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::ShowToolTip(const QString &toolTip)
{
	Q_UNUSED(toolTip)
	// do nothing
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::updateGroups()
{
    // do nothing
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::FileNew()
{
	if (individualMeasurements == nullptr)
	{
		NewMeasurementsDialog measurements(this);
		if (measurements.exec() == QDialog::Rejected)
		{
			return;
		}

		mUnit = measurements.measurementUnits();
		pUnit = mUnit;
		mType = measurements.type();

		data = new VContainer(qApp->translateVariables(), &mUnit);
		currentHeight = measurements.baseHeight();
		currentSize = measurements.baseSize();

		if (mType == MeasurementsType::Multisize)
		{
			individualMeasurements = new MeasurementDoc(mUnit, measurements.baseSize(), measurements.baseHeight(), data);
			individualMeasurements->setSize(&currentSize);
			individualMeasurements->setHeight(&currentHeight);
			m_curFileFormatVersion = MultiSizeConverter::MeasurementMaxVer;
			m_curFileFormatVersionStr = MultiSizeConverter::MeasurementMaxVerStr;
		}
		else
		{
			individualMeasurements = new MeasurementDoc(mUnit, data);
			m_curFileFormatVersion = IndividualSizeConverter::MeasurementMaxVer;
			m_curFileFormatVersionStr = IndividualSizeConverter::MeasurementMaxVerStr;
		}

		m_isReadOnly = individualMeasurements->isReadOnly();
		UpdatePadlock(m_isReadOnly);

		SetCurrentFile("");
		MeasurementsWasSaved(false);

		InitWindow();

		MeasurementGUI();
	}
	else
	{
		qApp->newMainWindow();
		qApp->mainWindow()->FileNew();
	}
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::OpenIndividual()
{
    const QString filter = tr("Individual measurements") + QLatin1String(" (*.") + smisExt +
                                                           QLatin1String(" *.") + vitExt  + QLatin1String(");;") +
                           tr("All files") + QLatin1String(" (*.*)");

	//Use standard path to individual measurements
	const QString dir = qApp->seamlyMeSettings()->getIndividualSizePath();

	bool usedNotExistedDir = false;
	QDir directory(dir);
	if (!directory.exists())
	{
		usedNotExistedDir = directory.mkpath(".");
	}

	Open(dir, filter);

	if (usedNotExistedDir)
	{
		QDir directory(dir);
		directory.rmpath(".");
	}
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::OpenMultisize()
{
    const QString filter = tr("Multisize measurements")  + QLatin1String(" (*.") + smmsExt +
                                                           QLatin1String(" *.") + vstExt  + QLatin1String(");;") +
                           tr("All files") + QLatin1String(" (*.*)");

	//Use standard path to multisize measurements
	QString dir = qApp->seamlyMeSettings()->getMultisizePath();
	dir = VCommonSettings::prepareMultisizeTables(dir);

	Open(dir, filter);
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::OpenTemplate()
{
    const QString filter = tr("Measurements") + QLatin1String(" (*.") + smisExt + QLatin1String(" *.") + smmsExt +
                                                QLatin1String(" *.") + vitExt + QLatin1String(" *.") + vstExt  +
                                                QLatin1String(");;") +
                           tr("All files")    + QLatin1String(" (*.*)");

	//Use standard path to template files
	QString dir = qApp->seamlyMeSettings()->getTemplatePath();
	dir = VCommonSettings::PrepareStandardTemplates(dir);
	Open(dir, filter);

	if (individualMeasurements != nullptr)
	{// The file was opened.
		SetCurrentFile(""); // Force user to to save new file
		lock.reset();// remove lock from template
	}
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::CreateFromExisting()
{
    const QString filter = tr("Individual measurements") + QLatin1String(" (*.") + smisExt +
                                                           QLatin1String(" *.") + vitExt + QLatin1String(")");

	//Use standard path to individual measurements
	const QString dir = qApp->seamlyMeSettings()->getIndividualSizePath();

	bool usedNotExistedDir = false;
	QDir directory(dir);
	if (!directory.exists())
	{
		usedNotExistedDir = directory.mkpath(".");
	}

    const QString filename = fileDialog(this, tr("Select file"), dir, filter, nullptr, QFileDialog::DontUseNativeDialog,
                                        QFileDialog::ExistingFile, QFileDialog::AcceptOpen);

	if (!filename.isEmpty())
	{
		if (individualMeasurements == nullptr)
		{
			LoadFromExistingFile(filename);
		}
		else
		{
			qApp->newMainWindow()->CreateFromExisting();
		}
	}

	if (usedNotExistedDir)
	{
		QDir directory(dir);
		directory.rmpath(".");
	}
}

/*
//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::handleBodyScanner1()
{
    const QString filter = QString("3D Measure Up") + QLatin1String(" (*.txt)");

    //Use standard path to template files
    QString dir = qApp->seamlyMeSettings()->getBodyScansPath();

    bool usedNotExistedDir = false;
    QDir directory(dir);
    if (!directory.exists())
    {
        usedNotExistedDir = directory.mkpath(".");
    }

    const QString filename = fileDialog(this, tr("Import body scan"), dir, filter, nullptr, QFileDialog::DontUseNativeDialog,
                                        QFileDialog::ExistingFile, QFileDialog::AcceptOpen);

    QMessageBox messageBox(this);
    messageBox.setMaximumWidth(600);
    messageBox.setText("3D Measure Up file:");
    messageBox.setInformativeText(filename);
    messageBox.setStandardButtons(QMessageBox::Ok);
    messageBox.setDefaultButton(QMessageBox::Ok);
    messageBox.exec();
}
*/

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::handleBodyScanner2()
{
    QString msg = tr("To utilize a 3DLook body scan the file needs to be converted to SeamlyME format.\n") +
                  tr("Attach your 3DLook file to an email and send to convert@seamly.io.\n\n") +
                  tr("You will recieve an email with the converted file, which you can then\nload in SeamlyME as usual.\n\n");

    QMessageBox messageBox(this);
    messageBox.setIconPixmap(QPixmap(":/icon/body_scan.png"));
    messageBox.setText(tr("Convert 3DLook file:"));
    messageBox.setInformativeText(msg);
    messageBox.setStandardButtons(QMessageBox::Ok);
    messageBox.setDefaultButton(QMessageBox::Ok);
    messageBox.exec();

/*
    VSettings *settings = new VSettings(QSettings::IniFormat, QSettings::UserScope, "Seamly2DTeam", "Seamly2D", this);

    QString to = "convert@seamly.io";
    QString name = settings->getCompanyName();
    QString email = settings->getEmail();
    QString subject = "3DLook file conversion";
    QString body = "Dear Seamly Team,\n\n Please convert the following 3DLook files to SeamlyME format.\n";

    QDesktopServices::openUrl(QUrl("mailto:" + to + "?subject=" + subject + "&body=" + body, QUrl::TolerantMode));
*/
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::Preferences()
{
	// Calling constructor of the dialog take some time. Because of this user have time to call the dialog twice.
	static QPointer<DialogSeamlyMePreferences> guard;// Prevent any second run
	if (guard.isNull())
	{
		QGuiApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
		DialogSeamlyMePreferences *preferences = new DialogSeamlyMePreferences(this);
		// QScopedPointer needs to be sure any exception will never block guard
		QScopedPointer<DialogSeamlyMePreferences> dlg(preferences);
		guard = preferences;
		// Must be first
		connect(dlg.data(), &DialogSeamlyMePreferences::updateProperties, this, &TMainWindow::WindowsLocale);
		connect(dlg.data(), &DialogSeamlyMePreferences::updateProperties, this, &TMainWindow::ToolBarStyles);
		QGuiApplication::restoreOverrideCursor();
		dlg->exec();
	}
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::ToolBarStyles()
{
	ToolBarStyle(ui->toolBarGradation);
	ToolBarStyle(ui->mainToolBar);
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::closeEvent(QCloseEvent *event)
{
	if (MaybeSave())
	{
		WriteSettings();
		event->accept();
		deleteLater();
	}
	else
	{
		event->ignore();
	}
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::changeEvent(QEvent *event)
{
	if (event->type() == QEvent::LanguageChange)
	{
		qApp->Settings()->getOsSeparator() ? setLocale(QLocale()) : setLocale(QLocale::c());

		// retranslate designer form (single inheritance approach)
		ui->retranslateUi(this);

		if (mType == MeasurementsType::Multisize)
		{
			ui->labelMType->setText(tr("Multisize measurements"));
			ui->labelBaseSizeValue->setText(QString().setNum(individualMeasurements->BaseSize()) + QLatin1String(" ") +
											UnitsToStr(individualMeasurements->measurementUnits(), true));
			ui->labelBaseHeightValue->setText(QString().setNum(individualMeasurements->BaseHeight()) + QLatin1String(" ") +
											  UnitsToStr(individualMeasurements->measurementUnits(), true));

			labelGradationHeights = new QLabel(tr("Height:"));
			labelGradationSizes = new QLabel(tr("Size:"));
		}
		else
		{
			ui->labelMType->setText(tr("Individual measurements"));

			const qint32 index = ui->comboBoxGender->currentIndex();
			ui->comboBoxGender->blockSignals(true);
			ui->comboBoxGender->clear();
			InitGender(ui->comboBoxGender);
			ui->comboBoxGender->setCurrentIndex(index);
			ui->comboBoxGender->blockSignals(false);
		}

		{
			const qint32 index = ui->comboBoxPMSystem->currentIndex();
			ui->comboBoxPMSystem->blockSignals(true);
			ui->comboBoxPMSystem->clear();
			InitPMSystems(ui->comboBoxPMSystem);
			ui->comboBoxPMSystem->setCurrentIndex(index);
			ui->comboBoxPMSystem->blockSignals(false);
		}

		{
			labelPatternUnit = new QLabel(tr("Pattern unit:"));

			if (comboBoxUnits != nullptr)
			{
				const qint32 index = comboBoxUnits->currentIndex();
				comboBoxUnits->blockSignals(true);
				comboBoxUnits->clear();
				InitComboBoxUnits();
				comboBoxUnits->setCurrentIndex(index);
				comboBoxUnits->blockSignals(false);
			}
		}
	}

	// remember to call base class implementation
	QMainWindow::changeEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::showEvent(QShowEvent *event)
{
	QMainWindow::showEvent( event );
	if ( event->spontaneous() )
	{
		return;
	}

	if (isInitialized)
	{
		return;
	}
	// do your init stuff here

	dockDiagramVisible = ui->dockWidgetDiagram->isVisible();
	ui->dockWidgetDiagram->setVisible(false);

	isInitialized = true;//first show windows are held
}

//---------------------------------------------------------------------------------------------------------------------
bool TMainWindow::eventFilter(QObject *object, QEvent *event)
{
	if (QPlainTextEdit *plainTextEdit = qobject_cast<QPlainTextEdit *>(object))
	{
		if (event->type() == QEvent::KeyPress)
		{
			QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
			if ((keyEvent->key() == Qt::Key_Enter) || (keyEvent->key() == Qt::Key_Return))
			{
				// Ignore Enter key
				return true;
			}
			else if ((keyEvent->key() == Qt::Key_Period) && (keyEvent->modifiers() & Qt::KeypadModifier))
			{
				if (qApp->Settings()->getOsSeparator())
				{
					plainTextEdit->insertPlainText(QLocale().decimalPoint());
				}
				else
				{
					plainTextEdit->insertPlainText(QLocale::c().decimalPoint());
				}
				return true;
			}
		}
	}
	else if (QLineEdit *textEdit = qobject_cast<QLineEdit *>(object))
	{
		if (event->type() == QEvent::KeyPress)
		{
			QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
			if ((keyEvent->key() == Qt::Key_Period) && (keyEvent->modifiers() & Qt::KeypadModifier))
			{
				if (qApp->Settings()->getOsSeparator())
				{
					textEdit->insert(QLocale().decimalPoint());
				}
				else
				{
					textEdit->insert(QLocale::c().decimalPoint());
				}
				return true;
			}
		}
	}
	else
	{
		// pass the event on to the parent class
		return QMainWindow::eventFilter(object, event);
	}
	return false;// pass the event to the widget
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::exportToCSVData(const QString &fileName, const DialogExportToCSV &dialog)
{
	QxtCsvModel csv;
    int columns;
    if (mType == MeasurementsType::Multisize)
    {
        columns = ui->tableWidget->columnCount();
    }
    else
    {
        columns = 5;
    }
	{
		int colCount = 0;
		for (int column = 0; column < columns; ++column)
		{
			if (!ui->tableWidget->isColumnHidden(column))
			{
				csv.insertColumn(colCount++);
			}
		}
	}

	if (dialog.WithHeader())
	{
		int colCount = 0;
		for (int column = 0; column < columns; ++column)
		{
			if (!ui->tableWidget->isColumnHidden(column))
			{
				QTableWidgetItem *header = ui->tableWidget->horizontalHeaderItem(colCount);
				csv.setHeaderText(colCount, header->text());
				++colCount;
			}
		}
	}

	const int rows = ui->tableWidget->rowCount();
	for (int row = 0; row < rows; ++row)
	{
		csv.insertRow(row);
		int colCount = 0;
		for (int column = 0; column < columns; ++column)
		{
			if (!ui->tableWidget->isColumnHidden(column))
			{
				QTableWidgetItem *item = ui->tableWidget->item(row, column);
				csv.setText(row, colCount, item->text());
				++colCount;
			}
		}
	}

	csv.toCSV(fileName, dialog.WithHeader(), dialog.Separator(), QTextCodec::codecForMib(dialog.SelectedMib()));
}

void TMainWindow::handleExportToCSV()
{
    QString file = tr("untitled");
    if(!curFile.isEmpty())
    {
        file = QFileInfo(curFile).baseName();
    }
    exportToCSV(file);
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::print()
{
    int width = 0;
    int height = 0;
    int columns;
    if (mType == MeasurementsType::Multisize)
    {
        columns = ui->tableWidget->columnCount();
    }
    else
    {
        columns = 5;
    }
    int rows = ui->tableWidget->rowCount();

    for( int i = 0; i < columns; ++i ) {
            width += ui->tableWidget->columnWidth(i);
    }

    for( int i = 0; i < rows; ++i ) {
        height += ui->tableWidget->rowHeight(i);
    }

    QPrintPreviewDialog  *dialog = new QPrintPreviewDialog(this);
    connect(dialog, &QPrintPreviewDialog::paintRequested, this, &TMainWindow::printPages);

    dialog->showMaximized();
    dialog->setWindowFlags(dialog->windowFlags() & ~Qt::WindowContextHelpButtonHint);
    dialog->exec();
}

void TMainWindow::printPages(QPrinter *printer)
{
    int columns;
    if (mType == MeasurementsType::Multisize)
    {
        columns = ui->tableWidget->columnCount();
    }
    else
    {
        columns = 5;
    }

    QTextDocument doc;

    QString text("<h2>" + CurrentFile() + "</h2>");
    text.append("<table>");
    if (mType == MeasurementsType::Multisize)
    {
        text.append("<tr><td align = left><b>Base Size:</b></td><td>" + ui->labelBaseSizeValue->text()     + "</td></tr>");
        text.append("<tr><td align = left><b>Base Height:</b></td><td>" + ui->labelBaseHeightValue->text() + "</td></tr>");
    }
    else
    {
        text.append("<tr><td align = left><b>Units:</b></td><td>"      + UnitsToStr(mUnit)                 + "</td></tr>");
        text.append("<tr><td align = left><b>First Name:</b></td><td>" + ui->lineEditGivenName->text()     + "</td></tr>");
        text.append("<tr><td align = left><b>Last Name:</b></td><td>"  + ui->lineEditFamilyName->text()    + "</td></tr>");
        text.append("<tr><td align = left><b>Gender:</b></td><td>"     + ui->comboBoxGender->currentText() + "</td></tr>");
        text.append("<tr><td align = left><b>Email:</b></td><td>"      + ui->lineEditEmail->text()         + "</td></tr>");
    }
    text.append("<tr><td align = left><b>Notes:</b></td><td>"      + ui->plainTextEditNotes->toPlainText() + "</td></tr></table>");
    text.append("<p>");

    text.append("<table><thead>");
    text.append("<tr>");
    for (int i = 0; i < columns; i++)
    {
        text.append("<th>").append(ui->tableWidget->horizontalHeaderItem(i)->data(Qt::DisplayRole).toString()).append("</th>");
    }
    text.append("</tr></thead>");
    text.append("<tbody>");
    for (int i = 0; i < ui->tableWidget->rowCount(); i++)
    {
        text.append("<tr>");
        for (int j = 0; j < columns; j++)
        {
            QTableWidgetItem *item = ui->tableWidget->item(i, j);
            if (!item || item->text().isEmpty())
            {

                if (j > 1)
                {
                    ui->tableWidget->setItem(i, j, new QTableWidgetItem("0"));
                }
                else
                {
                    ui->tableWidget->setItem(i, j, new QTableWidgetItem(""));
                }
            }
            if (j == 1 || j > 2)
            {
                text.append("<td align = center>").append(ui->tableWidget->item(i, j)->text()).append("</td>");
            }
            else
            {
                text.append("<td align = left>").append(ui->tableWidget->item(i, j)->text()).append("</td>");
            }
        }
        text.append("</tr>");
    }
    text.append("</tbody></table>");

        printer->setPageMargins(QMarginsF(10, 10, 10, 10), QPageLayout::Millimeter);
        doc.setHtml(text);
        doc.setPageSize(printer->pageLayout().paintRectPixels(static_cast<int>(PrintDPI)).size());
        doc.print(printer);
}

//---------------------------------------------------------------------------------------------------------------------
bool TMainWindow::FileSave()
{
	if (curFile.isEmpty() || m_isReadOnly)
	{
		return FileSaveAs();
	}
	else
	{
		if (mType == MeasurementsType::Multisize
				&& m_curFileFormatVersion < MultiSizeConverter::MeasurementMaxVer
				&& !ContinueFormatRewrite(m_curFileFormatVersionStr, MultiSizeConverter::MeasurementMaxVerStr))
		{
			return false;
		}
		else if (mType == MeasurementsType::Individual
				 && m_curFileFormatVersion < IndividualSizeConverter::MeasurementMaxVer
				 && !ContinueFormatRewrite(m_curFileFormatVersionStr, IndividualSizeConverter::MeasurementMaxVerStr))
		{
			return false;
		}

#ifdef Q_OS_WIN32
		qt_ntfs_permission_lookup++; // turn checking on
#endif /*Q_OS_WIN32*/
		const bool isFileWritable = QFileInfo(curFile).isWritable();
#ifdef Q_OS_WIN32
		qt_ntfs_permission_lookup--; // turn it off again
#endif /*Q_OS_WIN32*/

		if (!isFileWritable)
		{
			QMessageBox messageBox(this);
			messageBox.setIcon(QMessageBox::Question);
			messageBox.setText(tr("The measurements document has no write permissions."));
			messageBox.setInformativeText("Do you want to change the premissions?");
			messageBox.setStandardButtons(QMessageBox::Yes | QMessageBox::Cancel);
			messageBox.setDefaultButton(QMessageBox::Yes);

			if (messageBox.exec() == QMessageBox::Yes)
			{
#ifdef Q_OS_WIN32
				qt_ntfs_permission_lookup++; // turn checking on
#endif /*Q_OS_WIN32*/
				bool changed = QFile::setPermissions(curFile,
													 QFileInfo(curFile).permissions() | QFileDevice::WriteUser);
#ifdef Q_OS_WIN32
				qt_ntfs_permission_lookup--; // turn it off again
#endif /*Q_OS_WIN32*/

				if (!changed)
				{
					QMessageBox messageBox(this);
					messageBox.setIcon(QMessageBox::Warning);
					messageBox.setText(tr("Cannot set permissions for %1 to writable.").arg(curFile));
					messageBox.setInformativeText(tr("Could not save the file."));
					messageBox.setDefaultButton(QMessageBox::Ok);
					messageBox.setStandardButtons(QMessageBox::Ok);
					messageBox.exec();
					return false;
				}
			}
			else
			{
				return false;
			}
		}

		QString error;
		if (!SaveMeasurements(curFile, error))
		{
			QMessageBox messageBox;
			messageBox.setIcon(QMessageBox::Warning);
			messageBox.setText(tr("Could not save the file"));
			messageBox.setDefaultButton(QMessageBox::Ok);
			messageBox.setDetailedText(error);
			messageBox.setStandardButtons(QMessageBox::Ok);
			messageBox.exec();
			return false;
		}
		else
		{
			if (mType == MeasurementsType::Multisize)
			{
				m_curFileFormatVersion = MultiSizeConverter::MeasurementMaxVer;
				m_curFileFormatVersionStr = MultiSizeConverter::MeasurementMaxVerStr;
			}
			else
			{
				m_curFileFormatVersion = IndividualSizeConverter::MeasurementMaxVer;
				m_curFileFormatVersionStr = IndividualSizeConverter::MeasurementMaxVerStr;
			}
		}
	}
	return true;
}

//---------------------------------------------------------------------------------------------------------------------
bool TMainWindow::FileSaveAs()
{
	QString dir;
    QString filters;
    QString suffix;
    QString filePath = CurrentFile();
    QString fileName = QLatin1String("measurements");
    if (!filePath.isEmpty())
    {
        dir = QFileInfo(filePath).path();
        fileName = QFileInfo(filePath).baseName();
        if (mType == MeasurementsType::Individual)
        {

            filters = tr("Individual measurements") + QLatin1String(" (*.") + smisExt +
                                                      QLatin1String(" *.") + vitExt + QLatin1String(")");
            suffix = smisExt;
        }
        else
        {
            filters = tr("Multisize measurements") + QLatin1String(" (*.") + smmsExt +
                                                     QLatin1String(" *.") + vstExt + QLatin1String(")");
            suffix = smmsExt;
        }
    }
    else
    {
        if (mType == MeasurementsType::Individual)
        {
            dir = qApp->seamlyMeSettings()->getDefaultIndividualSizePath();
            filters = tr("Individual measurements") + QLatin1String(" (*.") + smisExt +
                                                      QLatin1String(" *.") + vitExt + QLatin1String(")");
            suffix = smisExt;
        }
        else
        {
            dir = qApp->seamlyMeSettings()->getDefaultMultisizePath();
            filters = tr("Multisize measurements") + QLatin1String(" (*.") + smmsExt +
                                                     QLatin1String(" *.") + vstExt + QLatin1String(")");
            suffix = smmsExt;
        }
    }

	if (mType == MeasurementsType::Individual)
	{
        filters = tr("Individual measurements") + QLatin1String(" (*.") + smisExt +
                                                  QLatin1String(" *.") + vitExt + QLatin1String(")");
		suffix = smisExt;
	}
	else
	{
		filters = tr("Multisize measurements") + QLatin1String(" (*.") + smmsExt +
                                                 QLatin1String(" *.") + vstExt + QLatin1String(")");
		suffix = smmsExt;
	}

    fileName += QLatin1String(".") + suffix;

	if (curFile.isEmpty())
	{
		if (mType == MeasurementsType::Individual)
		{
			dir = qApp->seamlyMeSettings()->getIndividualSizePath();
		}
		else
		{
			dir = qApp->seamlyMeSettings()->getMultisizePath();
			dir = VCommonSettings::prepareMultisizeTables(dir);
		}
	}
	else
	{
		dir = QFileInfo(curFile).absolutePath();
	}

	bool usedNotExistedDir = false;
	QDir directory(dir);
	if (!directory.exists())
	{
		usedNotExistedDir = directory.mkpath(".");
	}

    fileName = fileDialog(this, tr("Save as"), dir + QLatin1String("/") + fileName,
                                        filters, nullptr, QFileDialog::DontUseNativeDialog,
                                        QFileDialog::AnyFile, QFileDialog::AcceptSave);

	auto RemoveTempDir = [usedNotExistedDir, dir]()
	{
		if (usedNotExistedDir)
		{
			QDir directory(dir);
			directory.rmpath(".");
		}
	};

	if (fileName.isEmpty())
	{
		RemoveTempDir();
		return false;
	}

	QFileInfo fileInfo(fileName);
	if (fileInfo.suffix().isEmpty() && fileInfo.suffix() != suffix)
	{
		fileName += QLatin1String(".") + suffix;
	}

	if (fileInfo.exists() && fileName != filePath)
	{
		// Temporarily try to lock the file before saving
		VLockGuard<char> lock(fileName);
		if (!lock.IsLocked())
		{
			qCWarning(tMainWindow, "%s",
					   qUtf8Printable(tr("Failed to lock. This file already opened in another window.")));
			RemoveTempDir();
			return false;
		}
	}

	// Need for restoring previous state in case of failure
	const bool readOnly = individualMeasurements->isReadOnly();

	individualMeasurements->SetReadOnly(false);
	m_isReadOnly = false;

	QString error;
	bool result = SaveMeasurements(fileName, error);
	if (result == false)
	{
		QMessageBox messageBox;
		messageBox.setIcon(QMessageBox::Warning);
		messageBox.setInformativeText(tr("Could not save file"));
		messageBox.setDefaultButton(QMessageBox::Ok);
		messageBox.setDetailedText(error);
		messageBox.setStandardButtons(QMessageBox::Ok);
		messageBox.exec();

		// Restore previous state
		individualMeasurements->SetReadOnly(readOnly);
		m_isReadOnly = readOnly;
		RemoveTempDir();
		return false;
	}

	UpdatePadlock(false);
	UpdateWindowTitle();

    if (fileName != filePath)
    {
        VlpCreateLock(lock, fileName);
	    if (!lock->IsLocked())
        {
            qCCritical(tMainWindow, "%s", qUtf8Printable(tr("Failed to lock. This file already opened in another window. "
														    "Expect collisions when running 2 copies of the program.")));
		    RemoveTempDir();
	        return false;
	    }
    }

	RemoveTempDir();
	return true;
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::AboutToShowWindowMenu()
{
	ui->window_Menu->clear();
	CreateWindowMenu(ui->window_Menu);
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::ShowWindow() const
{
	if (QAction *action = qobject_cast<QAction*>(sender()))
	{
		const QVariant v = action->data();
		if (v.canConvert<int>())
		{
			const int offset = qvariant_cast<int>(v);
			const QList<TMainWindow*> windows = qApp->mainWindows();
			windows.at(offset)->raise();
			windows.at(offset)->activateWindow();
		}
	}
}

//---------------------------------------------------------------------------------------------------------------------
#if defined(Q_OS_MAC)
void TMainWindow::AboutToShowDockMenu()
{
	if (QMenu *menu = qobject_cast<QMenu *>(sender()))
	{
		menu->clear();
		CreateWindowMenu(menu);

		menu->addSeparator();

		menu->addAction(ui->actionOpenIndividual);
		menu->addAction(ui->actionOpenMultisize);
		menu->addAction(ui->actionOpenTemplate);

		menu->addSeparator();

		QAction *actionPreferences = menu->addAction(tr("Preferences"));
		actionPreferences->setMenuRole(QAction::NoRole);
		connect(actionPreferences, &QAction::triggered, this, &TMainWindow::Preferences);
	}
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::OpenAt(QAction *where)
{
	const QString path = curFile.left(curFile.indexOf(where->text())) + where->text();
	if (path == curFile)
	{
		return;
	}
	QProcess process;
	process.start(QStringLiteral("/usr/bin/open"), QStringList() << path, QIODevice::ReadOnly);
	process.waitForFinished();
}
#endif //defined(Q_OS_MAC)

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::SaveGivenName()
{
	if (individualMeasurements->GivenName() != ui->lineEditGivenName->text())
	{
		individualMeasurements->SetGivenName(ui->lineEditGivenName->text());
		MeasurementsWasSaved(false);
	}
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::SaveFamilyName()
{
	if (individualMeasurements->FamilyName() != ui->lineEditFamilyName->text())
	{
		individualMeasurements->SetFamilyName(ui->lineEditFamilyName->text());
		MeasurementsWasSaved(false);
	}
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::SaveEmail()
{
	if (individualMeasurements->Email() != ui->lineEditEmail->text())
	{
		individualMeasurements->SetEmail(ui->lineEditEmail->text());
		MeasurementsWasSaved(false);
	}
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::SaveGender(int index)
{
	const GenderType type = static_cast<GenderType>(ui->comboBoxGender->itemData(index).toInt());
	if (individualMeasurements->Gender() != type)
	{
		individualMeasurements->SetGender(type);
		MeasurementsWasSaved(false);
	}
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::SaveBirthDate(const QDate &date)
{
	if (individualMeasurements->BirthDate() != date)
	{
		individualMeasurements->SetBirthDate(date);
		MeasurementsWasSaved(false);
	}
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::SaveNotes()
{
	if (individualMeasurements->Notes() != ui->plainTextEditNotes->toPlainText())
	{
		individualMeasurements->SetNotes(ui->plainTextEditNotes->toPlainText());
		MeasurementsWasSaved(false);
	}
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::SavePMSystem(int index)
{
	QString system = ui->comboBoxPMSystem->itemData(index).toString();
	system.remove(0, 1);// clear p

	if (individualMeasurements->PMSystem() != system)
	{
		individualMeasurements->SetPMSystem(system);
		MeasurementsWasSaved(false);
	}
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::Remove()
{
	const int row = ui->tableWidget->currentRow();

	if (row == -1)
	{
		return;
	}

	const QTableWidgetItem *nameField = ui->tableWidget->item(ui->tableWidget->currentRow(), 0);
	individualMeasurements->Remove(nameField->data(Qt::UserRole).toString());

	MeasurementsWasSaved(false);

	search->RemoveRow(row);
	RefreshData();
	search->RefreshList(ui->lineEditFind->text());

	if (ui->tableWidget->rowCount() > 0)
	{
		ui->tableWidget->selectRow(row);
	}
	else
	{
		MFields(false);

		ui->actionExportToCSV->setEnabled(false);

		ui->lineEditName->blockSignals(true);
		ui->lineEditName->setText("");
		ui->lineEditName->blockSignals(false);

		ui->plainTextEditDescription->blockSignals(true);
		ui->plainTextEditDescription->setPlainText("");
		ui->plainTextEditDescription->blockSignals(false);

		ui->lineEditFullName->blockSignals(true);
		ui->lineEditFullName->setText("");
		ui->lineEditFullName->blockSignals(false);

		if (mType == MeasurementsType::Multisize)
		{
			ui->labelCalculatedValue->blockSignals(true);
			ui->doubleSpinBoxBaseValue->blockSignals(true);
			ui->doubleSpinBoxInSizes->blockSignals(true);
			ui->doubleSpinBoxInHeights->blockSignals(true);

			ui->labelCalculatedValue->setText("");
			ui->doubleSpinBoxBaseValue->setValue(0);
			ui->doubleSpinBoxInSizes->setValue(0);
			ui->doubleSpinBoxInHeights->setValue(0);

			ui->labelCalculatedValue->blockSignals(false);
			ui->doubleSpinBoxBaseValue->blockSignals(false);
			ui->doubleSpinBoxInSizes->blockSignals(false);
			ui->doubleSpinBoxInHeights->blockSignals(false);
		}
		else
		{
			ui->labelCalculatedValue->blockSignals(true);
			ui->labelCalculatedValue->setText("");
			ui->labelCalculatedValue->blockSignals(false);

			ui->plainTextEditFormula->blockSignals(true);
			ui->plainTextEditFormula->setPlainText("");
			ui->plainTextEditFormula->blockSignals(false);
		}
	}
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::MoveTop()
{
	const int row = ui->tableWidget->currentRow();

	if (row == -1)
	{
		return;
	}

	const QTableWidgetItem *nameField = ui->tableWidget->item(row, ColumnName);
	individualMeasurements->MoveTop(nameField->data(Qt::UserRole).toString());
	MeasurementsWasSaved(false);
	RefreshData();
	search->RefreshList(ui->lineEditFind->text());
	ui->tableWidget->selectRow(0);
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::MoveUp()
{
	const int row = ui->tableWidget->currentRow();

	if (row == -1)
	{
		return;
	}

	const QTableWidgetItem *nameField = ui->tableWidget->item(row, ColumnName);
	individualMeasurements->MoveUp(nameField->data(Qt::UserRole).toString());
	MeasurementsWasSaved(false);
	RefreshData();
	search->RefreshList(ui->lineEditFind->text());
	ui->tableWidget->selectRow(row-1);
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::MoveDown()
{
	const int row = ui->tableWidget->currentRow();

	if (row == -1)
	{
		return;
	}

	const QTableWidgetItem *nameField = ui->tableWidget->item(row, ColumnName);
	individualMeasurements->MoveDown(nameField->data(Qt::UserRole).toString());
	MeasurementsWasSaved(false);
	RefreshData();
	search->RefreshList(ui->lineEditFind->text());
	ui->tableWidget->selectRow(row+1);
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::MoveBottom()
{
	const int row = ui->tableWidget->currentRow();

	if (row == -1)
	{
		return;
	}

	const QTableWidgetItem *nameField = ui->tableWidget->item(row, ColumnName);
	individualMeasurements->MoveBottom(nameField->data(Qt::UserRole).toString());
	MeasurementsWasSaved(false);
	RefreshData();
	search->RefreshList(ui->lineEditFind->text());
	ui->tableWidget->selectRow(ui->tableWidget->rowCount()-1);
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::Fx()
{
	const int row = ui->tableWidget->currentRow();

	if (row == -1)
	{
		return;
	}

	const QTableWidgetItem *nameField = ui->tableWidget->item(row, ColumnName);

	QSharedPointer<MeasurementVariable> meash;

	try
	{
	   // Translate to internal look.
	   meash = data->getVariable<MeasurementVariable>(nameField->data(Qt::UserRole).toString());
	}

	catch(const VExceptionBadId &exception)
	{
		qCCritical(tMainWindow, "%s\n\n%s\n\n%s",
				   qUtf8Printable(tr("Can't find measurement '%1'.").arg(nameField->text())),
				   qUtf8Printable(exception.ErrorMessage()), qUtf8Printable(exception.DetailedInformation()));
		return;
	}

	EditFormulaDialog *dialog = new EditFormulaDialog(meash->GetData(), NULL_ID, MeasurementDialog, this);
	dialog->setWindowTitle(tr("Edit measurement"));
	dialog->SetFormula(qApp->translateVariables()->TryFormulaFromUser(ui->plainTextEditFormula->toPlainText().replace("\n", " "),
														  true));
	const QString postfix = UnitsToStr(mUnit, true);//Show unit in dialog label (cm, mm or inch)
	dialog->setPostfix(postfix);

	if (dialog->exec() == QDialog::Accepted)
	{
		// Because of the bug need to take QTableWidgetItem twice time. Previous update "killed" the pointer.
		const QTableWidgetItem *nameField = ui->tableWidget->item(row, ColumnName);
		individualMeasurements->SetMValue(nameField->data(Qt::UserRole).toString(), dialog->GetFormula());

		MeasurementsWasSaved(false);

		RefreshData();

		search->RefreshList(ui->lineEditFind->text());

		ui->tableWidget->selectRow(row);
	}
	delete dialog;
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::AddCustom()
{
	const QString name = GetCustomName();
	qint32 currentRow = -1;

	if (ui->tableWidget->currentRow() == -1)
	{
		currentRow  = ui->tableWidget->rowCount();
		individualMeasurements->addEmpty(name);
	}
	else
	{
		currentRow  = ui->tableWidget->currentRow()+1;
		const QTableWidgetItem *nameField = ui->tableWidget->item(ui->tableWidget->currentRow(), ColumnName);
		individualMeasurements->AddEmptyAfter(nameField->data(Qt::UserRole).toString(), name);
	}

	search->AddRow(currentRow);
	RefreshData();
	search->RefreshList(ui->lineEditFind->text());

	ui->tableWidget->selectRow(currentRow);

	ui->actionExportToCSV->setEnabled(true);

	MeasurementsWasSaved(false);
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::AddKnown()
{
	QScopedPointer<MeasurementDatabaseDialog> dialog (new MeasurementDatabaseDialog(individualMeasurements->listKnown(), this));
	if (dialog->exec() == QDialog::Accepted)
	{
		qint32 currentRow;

		const QStringList list = dialog->getNewMeasurementNames();
		if (ui->tableWidget->currentRow() == -1)
		{
			currentRow  = ui->tableWidget->rowCount() + list.size() - 1;
			for (int i = 0; i < list.size(); ++i)
			{
				if (mType == MeasurementsType::Individual)
				{
					individualMeasurements->addEmpty(list.at(i), qApp->translateVariables()->MFormula(list.at(i)));
				}
				else
				{
					individualMeasurements->addEmpty(list.at(i));
				}

				search->AddRow(currentRow);
			}
		}
		else
		{
			currentRow  = ui->tableWidget->currentRow() + list.size();
			const QTableWidgetItem *nameField = ui->tableWidget->item(ui->tableWidget->currentRow(), ColumnName);
			QString after = nameField->data(Qt::UserRole).toString();
			for (int i = 0; i < list.size(); ++i)
			{
				if (mType == MeasurementsType::Individual)
				{
					individualMeasurements->AddEmptyAfter(after, list.at(i), qApp->translateVariables()->MFormula(list.at(i)));
				}
				else
				{
					individualMeasurements->AddEmptyAfter(after, list.at(i));
				}
				search->AddRow(currentRow);
				after = list.at(i);
			}
		}

		RefreshData();
		search->RefreshList(ui->lineEditFind->text());

		ui->tableWidget->selectRow(currentRow);

		ui->actionExportToCSV->setEnabled(true);

		MeasurementsWasSaved(false);
	}
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::ImportFromPattern()
{
	if (individualMeasurements == nullptr)
	{
		return;
	}

    const QString filter = tr("Pattern files") + QLatin1String(" (*.") + valExt +
                           QLatin1String(" *.") + sm2dExt + QLatin1String(")");
	//Use standard path to individual measurements
	QString dir = qApp->seamlyMeSettings()->getTemplatePath();
	dir = VCommonSettings::PrepareStandardTemplates(dir);

    const QString filename = fileDialog(this, tr("Import from a pattern"), dir, filter, nullptr,
                                        QFileDialog::DontUseNativeDialog, QFileDialog::ExistingFile,
                                        QFileDialog::AcceptOpen);

	if (filename.isEmpty())
	{
		return;
	}

	VLockGuard<char> lock(filename);
	if (!lock.IsLocked())
	{
		qCCritical(tMainWindow, "%s", qUtf8Printable(tr("This file already opened in another window.")));
		return;
	}

	QStringList measurements;
	try
	{
		VPatternConverter converter(filename);
		QScopedPointer<VLitePattern> doc(new VLitePattern());
		doc->setXMLContent(converter.Convert());
		measurements = doc->ListMeasurements();
	}

	catch (VException &exception)
	{
		qCCritical(tMainWindow, "%s\n\n%s\n\n%s", qUtf8Printable(tr("File error.")),
				   qUtf8Printable(exception.ErrorMessage()), qUtf8Printable(exception.DetailedInformation()));
		return;
	}

	measurements = FilterMeasurements(measurements, individualMeasurements->ListAll());

	qint32 currentRow;

	if (ui->tableWidget->currentRow() == -1)
	{
		currentRow  = ui->tableWidget->rowCount() + measurements.size() - 1;
		for (int i = 0; i < measurements.size(); ++i)
		{
			individualMeasurements->addEmpty(measurements.at(i));
		}
	}
	else
	{
		currentRow  = ui->tableWidget->currentRow() + measurements.size();
		const QTableWidgetItem *nameField = ui->tableWidget->item(ui->tableWidget->currentRow(), ColumnName);
		QString after = nameField->data(Qt::UserRole).toString();
		for (int i = 0; i < measurements.size(); ++i)
		{
			individualMeasurements->AddEmptyAfter(after, measurements.at(i));
			after = measurements.at(i);
		}
	}

	RefreshData();

	search->RefreshList(ui->lineEditFind->text());

	ui->tableWidget->selectRow(currentRow);

	MeasurementsWasSaved(false);
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::ChangedSize(int index)
{
	const int row = ui->tableWidget->currentRow();
    currentSize = gradationSizes->itemText(index).toInt();
	RefreshData();
	search->RefreshList(ui->lineEditFind->text());
	ui->tableWidget->selectRow(row);
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::ChangedHeight(int index)
{
	const int row = ui->tableWidget->currentRow();
    currentHeight = gradationHeights->itemText(index).toInt();
	RefreshData();
	search->RefreshList(ui->lineEditFind->text());
	ui->tableWidget->selectRow(row);
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::ShowMData()
{
	ShowNewMData(true);
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::ShowNewMData(bool fresh)
{
	if (ui->tableWidget->rowCount() > 0)
	{
		MFields(true);

		const QTableWidgetItem *nameField = ui->tableWidget->item(ui->tableWidget->currentRow(), ColumnName); // name
		QSharedPointer<MeasurementVariable> meash;

		try
		{
			// Translate to internal look.
			meash = data->getVariable<MeasurementVariable>(nameField->data(Qt::UserRole).toString());
		}

		catch(const VExceptionBadId &exception)
		{
			Q_UNUSED(exception)
			MFields(false);
			return;
		}

		ShowMDiagram(meash->GetName());

		// Don't block all signal for QLineEdit. Need for correct handle with clear button.
		disconnect(ui->lineEditName, &QLineEdit::textEdited, this, &TMainWindow::SaveMName);
		ui->plainTextEditDescription->blockSignals(true);
		if (meash->isCustom())
		{
			ui->plainTextEditDescription->setPlainText(meash->GetDescription());
			ui->lineEditFullName->setText(meash->getGuiText());
			ui->lineEditName->setText(ClearCustomName(meash->GetName()));
		}
		else
		{
			//Show known
			ui->plainTextEditDescription->setPlainText(qApp->translateVariables()->Description(meash->GetName()));
			ui->lineEditFullName->setText(qApp->translateVariables()->guiText(meash->GetName()));
			ui->lineEditName->setText(nameField->text());
		}
		connect(ui->lineEditName, &QLineEdit::textEdited, this, &TMainWindow::SaveMName);
		ui->plainTextEditDescription->blockSignals(false);

		if (mType == MeasurementsType::Multisize)
		{
			ui->labelCalculatedValue->blockSignals(true);
			ui->doubleSpinBoxBaseValue->blockSignals(true);
			ui->doubleSpinBoxInSizes->blockSignals(true);
			ui->doubleSpinBoxInHeights->blockSignals(true);

			const QString postfix = UnitsToStr(pUnit);//Show unit in dialog label (cm, mm or inch)
			const qreal value = UnitConvertor(*data->DataVariables()->value(meash->GetName())->GetValue(), mUnit,
											  pUnit);
			ui->labelCalculatedValue->setText(qApp->LocaleToString(value) + " " +postfix);

			if (fresh)
			{
				ui->doubleSpinBoxBaseValue->setValue(meash->GetBase());
				ui->doubleSpinBoxInSizes->setValue(meash->GetKsize());
				ui->doubleSpinBoxInHeights->setValue(meash->GetKheight());
			}

			ui->labelCalculatedValue->blockSignals(false);
			ui->doubleSpinBoxBaseValue->blockSignals(false);
			ui->doubleSpinBoxInSizes->blockSignals(false);
			ui->doubleSpinBoxInHeights->blockSignals(false);
		}
		else
		{
			EvalFormula(meash->GetFormula(), false, meash->GetData(), ui->labelCalculatedValue);

			ui->plainTextEditFormula->blockSignals(true);

			QString formula;
			try
			{
				formula = qApp->translateVariables()->FormulaToUser(meash->GetFormula(), qApp->Settings()->getOsSeparator());
			}
			catch (qmu::QmuParserError &error)
			{
				Q_UNUSED(error)
				formula = meash->GetFormula();
			}

			ui->plainTextEditFormula->setPlainText(formula);
			ui->plainTextEditFormula->blockSignals(false);
		}

		MeasurementGUI();
	}
	else
	{
		MFields(false);
	}
}

//---------------------------------------------------------------------------------------------------------------------
QString TMainWindow::getMeasurementNumber(const QString &name)
{
	return  qApp->translateVariables()->MNumber(name);
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::ShowMDiagram(const QString &name)
{
	const VTranslateVars *trv = qApp->translateVariables();
	const QString number = trv->MNumber(name);

	if (number.isEmpty())
	{
		ui->labelDiagram->setText(tr("<html><head/><body><p><span style=\" font-size:340pt;\">?</span></p>"
									 "<p align=\"center\">Unknown measurement</p></body></html>"));
	}
	else
	{
		ui->labelDiagram->setText(QString("<html><head/><body><p align=\"center\">%1</p>"
										  "<p align=\"center\"><b>%2</b>. <i>%3</i></p></body></html>")
										  .arg(MeasurementDatabaseDialog::imageUrl(number), number, trv->guiText(name)));
	}
	// This part is very ugly, can't find better way to resize dockWidget.
	ui->labelDiagram->adjustSize();
	// And also those 50 px. DockWidget has some border. And i can't find how big it is.
	// Can lead to problem in future.
	ui->dockWidgetDiagram->setMaximumWidth(ui->labelDiagram->width()+50);
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::SaveMName(const QString &text)
{
	const int row = ui->tableWidget->currentRow();

	if (row == -1)
	{
		return;
	}

	const QTableWidgetItem *nameField = ui->tableWidget->item(ui->tableWidget->currentRow(), ColumnName);

	QSharedPointer<MeasurementVariable> meash;

	try
	{
		// Translate to internal look.
		meash = data->getVariable<MeasurementVariable>(nameField->data(Qt::UserRole).toString());
	}

	catch(const VExceptionBadId &exception)
	{
		qCWarning(tMainWindow, "%s\n\n%s\n\n%s",
				  qUtf8Printable(tr("Can't find measurement '%1'.").arg(nameField->text())),
				  qUtf8Printable(exception.ErrorMessage()), qUtf8Printable(exception.DetailedInformation()));
		return;
	}

	QString newName = text;

	if (meash->isCustom())
	{
		newName.isEmpty() ? newName = GetCustomName() : newName = CustomMSign + newName;

		if (!data->IsUnique(newName))
		{
			qint32 num = 2;
			QString name = newName;
			do
			{
				name = name + QLatin1String("_") + QString().number(num);
				num++;
			} while (!data->IsUnique(name));
			newName = name;
		}

		individualMeasurements->SetMName(nameField->text(), newName);
		MeasurementsWasSaved(false);
		RefreshData();
		search->RefreshList(ui->lineEditFind->text());

		ui->tableWidget->blockSignals(true);
		ui->tableWidget->selectRow(row);
		ui->tableWidget->blockSignals(false);
	}
	else
	{
		qCWarning(tMainWindow, "%s", qUtf8Printable(tr("The name of known measurement forbidden to change.")));
	}
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::SaveMValue()
{
	const int row = ui->tableWidget->currentRow();

	if (row == -1)
	{
		return;
	}

	const QTableWidgetItem *nameField = ui->tableWidget->item(row, ColumnName);

	// Replace line return character with spaces for calc if exist
	QString text = ui->plainTextEditFormula->toPlainText();
	text.replace("\n", " ");

	QTableWidgetItem *formulaField = ui->tableWidget->item(row, ColumnFormula);
	if (formulaField->text() == text)
	{
		QTableWidgetItem *result = ui->tableWidget->item(row, ColumnCalcValue);
		const QString postfix = UnitsToStr(mUnit);//Show unit in dialog label (cm, mm or inch)
		ui->labelCalculatedValue->setText(result->text() + " " +postfix);
		return;
	}

	if (text.isEmpty())
	{
		const QString postfix = UnitsToStr(mUnit);//Show unit in dialog label (cm, mm or inch)
		ui->labelCalculatedValue->setText(tr("Error") + " (" + postfix + "). " + tr("Empty field."));
		return;
	}

	QSharedPointer<MeasurementVariable> meash;
	try
	{
		// Translate to internal look.
		meash = data->getVariable<MeasurementVariable>(nameField->data(Qt::UserRole).toString());
	}

	catch(const VExceptionBadId &exception)
	{
		qCWarning(tMainWindow, "%s\n\n%s\n\n%s",
				  qUtf8Printable(tr("Can't find measurement '%1'.").arg(nameField->text())),
				  qUtf8Printable(exception.ErrorMessage()), qUtf8Printable(exception.DetailedInformation()));
		return;
	}

	if (!EvalFormula(text, true, meash->GetData(), ui->labelCalculatedValue))
	{
		return;
	}

	try
	{
		const QString formula = qApp->translateVariables()->FormulaFromUser(text, qApp->Settings()->getOsSeparator());
		individualMeasurements->SetMValue(nameField->data(Qt::UserRole).toString(), formula);
	}
	catch (qmu::QmuParserError &error) // Just in case something bad will happen
	{
		Q_UNUSED(error)
		return;
	}

	MeasurementsWasSaved(false);

	const QTextCursor cursor = ui->plainTextEditFormula->textCursor();

	RefreshData();
	search->RefreshList(ui->lineEditFind->text());

	ui->tableWidget->blockSignals(true);
	ui->tableWidget->selectRow(row);
	ui->tableWidget->blockSignals(false);

	ui->plainTextEditFormula->setTextCursor(cursor);
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::SaveMBaseValue(double value)
{
	const int row = ui->tableWidget->currentRow();

	if (row == -1)
	{
		return;
	}

	const QTableWidgetItem *nameField = ui->tableWidget->item(ui->tableWidget->currentRow(), ColumnName);
	individualMeasurements->SetMBaseValue(nameField->data(Qt::UserRole).toString(), value);

	MeasurementsWasSaved(false);

	RefreshData();
	search->RefreshList(ui->lineEditFind->text());

	ui->tableWidget->blockSignals(true);
	ui->tableWidget->selectRow(row);
	ui->tableWidget->blockSignals(false);

	ShowNewMData(false);
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::SaveMSizeIncrease(double value)
{
	const int row = ui->tableWidget->currentRow();

	if (row == -1)
	{
		return;
	}

	const QTableWidgetItem *nameField = ui->tableWidget->item(ui->tableWidget->currentRow(), ColumnName);
	individualMeasurements->SetMSizeIncrease(nameField->data(Qt::UserRole).toString(), value);

	MeasurementsWasSaved(false);

	RefreshData();
	search->RefreshList(ui->lineEditFind->text());

	ui->tableWidget->blockSignals(true);
	ui->tableWidget->selectRow(row);
	ui->tableWidget->blockSignals(false);

	ShowNewMData(false);
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::SaveMHeightIncrease(double value)
{
	const int row = ui->tableWidget->currentRow();

	if (row == -1)
	{
		return;
	}

	const QTableWidgetItem *nameField = ui->tableWidget->item(ui->tableWidget->currentRow(), ColumnName);
	individualMeasurements->SetMHeightIncrease(nameField->data(Qt::UserRole).toString(), value);

	MeasurementsWasSaved(false);

	RefreshData();
	search->RefreshList(ui->lineEditFind->text());

	ui->tableWidget->blockSignals(true);
	ui->tableWidget->selectRow(row);
	ui->tableWidget->blockSignals(false);

	ShowNewMData(false);
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::SaveMDescription()
{
	const int row = ui->tableWidget->currentRow();

	if (row == -1)
	{
		return;
	}

	const QTableWidgetItem *nameField = ui->tableWidget->item(ui->tableWidget->currentRow(), ColumnName);
	individualMeasurements->SetMDescription(nameField->data(Qt::UserRole).toString(), ui->plainTextEditDescription->toPlainText());

	MeasurementsWasSaved(false);

	const QTextCursor cursor = ui->plainTextEditDescription->textCursor();

	RefreshData();

	ui->tableWidget->blockSignals(true);
	ui->tableWidget->selectRow(row);
	ui->tableWidget->blockSignals(false);

	ui->plainTextEditDescription->setTextCursor(cursor);
}


//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::SaveMFullName()
{
	const int row = ui->tableWidget->currentRow();

	if (row == -1)
	{
		return;
	}

	const QTableWidgetItem *nameField = ui->tableWidget->item(ui->tableWidget->currentRow(), ColumnName);

	QSharedPointer<MeasurementVariable> meash;

	try
	{
		// Translate to internal look.
		meash = data->getVariable<MeasurementVariable>(nameField->data(Qt::UserRole).toString());
	}

	catch(const VExceptionBadId &exception)
	{
		qCWarning(tMainWindow, "%s\n\n%s\n\n%s",
				  qUtf8Printable(tr("Can't find measurement '%1'.").arg(nameField->text())),
				  qUtf8Printable(exception.ErrorMessage()), qUtf8Printable(exception.DetailedInformation()));
		return;
	}

	if (meash->isCustom())
	{
		individualMeasurements->SetMFullName(nameField->data(Qt::UserRole).toString(), ui->lineEditFullName->text());

		MeasurementsWasSaved(false);

		RefreshData();

		ui->tableWidget->blockSignals(true);
		ui->tableWidget->selectRow(row);
		ui->tableWidget->blockSignals(false);
	}
	else
	{
		qCWarning(tMainWindow, "%s", qUtf8Printable(tr("The full name of known measurement forbidden to change.")));
	}
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::PatternUnitChanged(int index)
{
	pUnit = static_cast<Unit>(comboBoxUnits->itemData(index).toInt());

	UpdatePatternUnit();
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::SetupMenu()
{
	// File
	connect(ui->actionNew, &QAction::triggered, this, &TMainWindow::FileNew);
	connect(ui->actionOpenIndividual, &QAction::triggered, this, &TMainWindow::OpenIndividual);
	connect(ui->actionOpenMultisize, &QAction::triggered, this, &TMainWindow::OpenMultisize);
	connect(ui->actionOpenTemplate, &QAction::triggered, this, &TMainWindow::OpenTemplate);
	connect(ui->actionCreateFromExisting, &QAction::triggered, this, &TMainWindow::CreateFromExisting);

    //connect(ui->bodyScanner1_Action, &QAction::triggered, this, &TMainWindow::handleBodyScanner1);
	connect(ui->bodyScanner2_Action, &QAction::triggered, this, &TMainWindow::handleBodyScanner2);

	connect(ui->print_Action, &QAction::triggered, this, &TMainWindow::print);
    connect(ui->actionSave, &QAction::triggered, this, &TMainWindow::FileSave);
	connect(ui->actionSaveAs, &QAction::triggered, this, &TMainWindow::FileSaveAs);
	connect(ui->actionExportToCSV, &QAction::triggered, this, &TMainWindow::handleExportToCSV);
	connect(ui->actionReadOnly, &QAction::triggered, this, [this](bool ro)
	{
		if (!m_isReadOnly)
		{
			individualMeasurements->SetReadOnly(ro);
			MeasurementsWasSaved(false);
			UpdatePadlock(ro);
			UpdateWindowTitle();
		}
		else
		{
			if (QAction *action = qobject_cast< QAction * >(this->sender()))
			{
				action->setChecked(true);
			}
		}
	});
	connect(ui->actionPreferences, &QAction::triggered, this, &TMainWindow::Preferences);

	for (int i = 0; i < MaxRecentFiles; ++i)
	{
		QAction *action = new QAction(this);
		recentFileActs[i] = action;
		connect(action, &QAction::triggered, this, [this]()
		{
			QAction *action = qobject_cast<QAction *>(sender());
			if (action)
			{
				const QString filePath = action->data().toString();
				if (!filePath.isEmpty())
				{
					LoadFile(filePath);
				}
			}
		});
		ui->menuFile->insertAction(ui->actionPreferences, recentFileActs[i]);
		recentFileActs[i]->setVisible(false);
	}

	separatorAct = new QAction(this);
	separatorAct->setSeparator(true);
	separatorAct->setVisible(false);
	ui->menuFile->insertAction(ui->actionPreferences, separatorAct );


	connect(ui->actionQuit, &QAction::triggered, this, &TMainWindow::close);

	// Measurements
	connect(ui->actionAddCustom, &QAction::triggered, this, &TMainWindow::AddCustom);
	connect(ui->actionAddKnown, &QAction::triggered, this, &TMainWindow::AddKnown);
	connect(ui->actionDatabase, &QAction::triggered, qApp, &ApplicationME::showDataBase);
	connect(ui->actionImportFromPattern, &QAction::triggered, this, &TMainWindow::ImportFromPattern);
	actionDockDiagram = ui->dockWidgetDiagram->toggleViewAction();
	actionDockDiagram->setMenuRole(QAction::NoRole);
	ui->measurements_Menu->addAction(actionDockDiagram);
	ui->mainToolBar->addAction(actionDockDiagram);
	actionDockDiagram->setEnabled(false);
	actionDockDiagram->setIcon(QIcon("://seamlymeicon/24x24/mannequin.png"));

	// Window
	connect(ui->window_Menu, &QMenu::aboutToShow, this, &TMainWindow::AboutToShowWindowMenu);
	AboutToShowWindowMenu();

	// Help
    connect(ui->shortcuts_Action, &QAction::triggered, this, [this]()
    {
        MeShortcutsDialog *shortcutsDialog = new MeShortcutsDialog(this);
        shortcutsDialog->setAttribute(Qt::WA_DeleteOnClose, true);
        shortcutsDialog->show();
    });
	connect(ui->actionAboutQt, &QAction::triggered, this, [this]()
	{
		QMessageBox::aboutQt(this, tr("About Qt"));
	});
	connect(ui->actionAboutSeamlyMe, &QAction::triggered, this, [this]()
	{
		DialogAboutSeamlyMe *aboutDialog = new DialogAboutSeamlyMe(this);
		aboutDialog->setAttribute(Qt::WA_DeleteOnClose, true);
		aboutDialog->show();
	});

	//Actions for recent files loaded by a seamlyme window application.
	UpdateRecentFileActions();

}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::InitWindow()
{
	SCASSERT(individualMeasurements != nullptr)
	ui->labelToolTip->setVisible(false);
	ui->tabWidget->setVisible(true);
	ui->dockWidgetDiagram->setVisible(dockDiagramVisible);
	actionDockDiagram->setEnabled(true);
	ui->tabWidget->setCurrentIndex(0);

	ui->plainTextEditNotes->setEnabled(true);
	ui->toolBarGradation->setVisible(true);

	if (mType == MeasurementsType::Multisize)
	{
		ui->labelMType->setText(tr("Multisize measurements"));
		ui->labelBaseSizeValue->setText(QString().setNum(individualMeasurements->BaseSize()) + " " +
										UnitsToStr(individualMeasurements->measurementUnits(), true));
		ui->labelBaseHeightValue->setText(QString().setNum(individualMeasurements->BaseHeight()) + " " +
										  UnitsToStr(individualMeasurements->measurementUnits(), true));

		// Because Qt Designer doesn't know about our deleting we will create empty objects for correct
		// working the retranslation UI
		// Tab Measurements
		HackWidget(&ui->plainTextEditFormula);
		HackWidget(&ui->toolButtonExpr);
		HackWidget(&ui->labelFormula);

		// Tab Information
		HackWidget(&ui->lineEditGivenName);
		HackWidget(&ui->lineEditFamilyName);
		HackWidget(&ui->comboBoxGender);
		HackWidget(&ui->lineEditEmail);
		HackWidget(&ui->labelGivenName);
		HackWidget(&ui->labelFamilyName);
		HackWidget(&ui->labelBirthDate);
		HackWidget(&ui->dateEditBirthDate);
		HackWidget(&ui->labelGender);
		HackWidget(&ui->labelEmail);

		const QStringList listHeights = MeasurementVariable::WholeListHeights(mUnit);
		const QStringList listSizes = MeasurementVariable::WholeListSizes(mUnit);

		labelGradationHeights = new QLabel(tr("Height:"));
		gradationHeights = SetGradationList(labelGradationHeights, listHeights);
		SetDefaultHeight(static_cast<int>(VContainer::height()));
		connect(gradationHeights, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
                this, &TMainWindow::ChangedHeight);

		labelGradationSizes = new QLabel(tr("Size:"));
		gradationSizes = SetGradationList(labelGradationSizes, listSizes);
		SetDefaultSize(static_cast<int>(VContainer::size()));
		connect(gradationSizes, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
                this, &TMainWindow::ChangedSize);

		connect(ui->doubleSpinBoxBaseValue,
				static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
				this, &TMainWindow::SaveMBaseValue);
		connect(ui->doubleSpinBoxInSizes,
				static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
				this, &TMainWindow::SaveMSizeIncrease);
		connect(ui->doubleSpinBoxInHeights,
				static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
				this, &TMainWindow::SaveMHeightIncrease);

		SetDecimals();
	}
	else
	{
		ui->labelMType->setText(tr("Individual measurements"));

		ui->lineEditGivenName->setEnabled(true);
		ui->lineEditFamilyName->setEnabled(true);
		ui->dateEditBirthDate->setEnabled(true);
		ui->comboBoxGender->setEnabled(true);
		ui->lineEditEmail->setEnabled(true);

		// Tab Measurements
		HackWidget(&ui->doubleSpinBoxBaseValue);
		HackWidget(&ui->doubleSpinBoxInSizes);
		HackWidget(&ui->doubleSpinBoxInHeights);
		HackWidget(&ui->labelBaseValue);
		HackWidget(&ui->labelInSizes);
		HackWidget(&ui->labelInHeights);

		// Tab Information
		HackWidget(&ui->labelBaseSize);
		HackWidget(&ui->labelBaseSizeValue);
		HackWidget(&ui->labelBaseHeight);
		HackWidget(&ui->labelBaseHeightValue);

		ui->lineEditGivenName->setText(individualMeasurements->GivenName());
		ui->lineEditFamilyName->setText(individualMeasurements->FamilyName());

		ui->comboBoxGender->clear();
		InitGender(ui->comboBoxGender);
		const qint32 index = ui->comboBoxGender->findData(static_cast<int>(individualMeasurements->Gender()));
		ui->comboBoxGender->setCurrentIndex(index);

		{
			const QLocale dateLocale = QLocale(qApp->Settings()->getLocale());
			ui->dateEditBirthDate->setLocale(dateLocale);
			ui->dateEditBirthDate->setDisplayFormat(dateLocale.dateFormat());
			ui->dateEditBirthDate->setDate(individualMeasurements->BirthDate());
		}

		ui->lineEditEmail->setText(individualMeasurements->Email());

		connect(ui->lineEditGivenName, &QLineEdit::editingFinished, this, &TMainWindow::SaveGivenName);
		connect(ui->lineEditFamilyName, &QLineEdit::editingFinished, this, &TMainWindow::SaveFamilyName);
		connect(ui->lineEditEmail, &QLineEdit::editingFinished, this, &TMainWindow::SaveEmail);
		connect(ui->comboBoxGender, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this,
				&TMainWindow::SaveGender);
		connect(ui->dateEditBirthDate, &QDateEdit::dateChanged, this, &TMainWindow::SaveBirthDate);

		connect(ui->plainTextEditFormula, &QPlainTextEdit::textChanged, this, &TMainWindow::SaveMValue,
				Qt::UniqueConnection);

		connect(ui->toolButtonExpr, &QToolButton::clicked, this, &TMainWindow::Fx);
	}

	ui->comboBoxPMSystem->setEnabled(true);
	ui->comboBoxPMSystem->clear();
	InitPMSystems(ui->comboBoxPMSystem);
	const qint32 index = ui->comboBoxPMSystem->findData(QLatin1Char('p')+individualMeasurements->PMSystem());
	ui->comboBoxPMSystem->setCurrentIndex(index);
	connect(ui->comboBoxPMSystem, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this,
			&TMainWindow::SavePMSystem);

	connect(ui->lineEditFind, &QLineEdit::textChanged, [this] (const QString &term){search->Find(term);});
	connect(ui->toolButtonFindPrevious, &QToolButton::clicked, [this] (){search->FindPrevious();});
	connect(ui->toolButtonFindNext, &QToolButton::clicked, [this] (){search->FindNext();});

	connect(search.data(), &VTableSearch::HasResult, this, [this] (bool state)
	{
		ui->toolButtonFindPrevious->setEnabled(state);
	});
    connect(ui->clipboard_ToolButton, &QToolButton::clicked, this, &TMainWindow::copyToClipboard);
	connect(search.data(), &VTableSearch::HasResult, this, [this] (bool state)
	{
		ui->toolButtonFindNext->setEnabled(state);
	});

	ui->plainTextEditNotes->setPlainText(individualMeasurements->Notes());
	connect(ui->plainTextEditNotes, &QPlainTextEdit::textChanged, this, &TMainWindow::SaveNotes);

	ui->actionAddCustom->setEnabled(true);
	ui->actionAddKnown->setEnabled(true);
	ui->actionImportFromPattern->setEnabled(true);
	ui->actionSaveAs->setEnabled(true);

	ui->lineEditName->setValidator(new QRegularExpressionValidator(QRegularExpression(
																	   QLatin1String("^$|")+NameRegExp()),
																   this));

	connect(ui->toolButtonRemove, &QToolButton::clicked, this, &TMainWindow::Remove);
	connect(ui->toolButtonTop, &QToolButton::clicked, this, &TMainWindow::MoveTop);
	connect(ui->toolButtonUp, &QToolButton::clicked, this, &TMainWindow::MoveUp);
	connect(ui->toolButtonDown, &QToolButton::clicked, this, &TMainWindow::MoveDown);
	connect(ui->toolButtonBottom, &QToolButton::clicked, this, &TMainWindow::MoveBottom);

	connect(ui->lineEditName, &QLineEdit::textEdited, this, &TMainWindow::SaveMName);
	connect(ui->plainTextEditDescription, &QPlainTextEdit::textChanged, this, &TMainWindow::SaveMDescription);
	connect(ui->lineEditFullName, &QLineEdit::textEdited, this, &TMainWindow::SaveMFullName);

	connect(ui->pushButtonShowInExplorer, &QPushButton::clicked, this, [this]()
	{
		ShowInGraphicalShell(curFile);
	});

	initUnits();

	initializeTable();
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::initializeTable()
{
	if (mType == MeasurementsType::Multisize)
	{
		ui->tableWidget->setColumnHidden( ColumnFormula, true );// formula
	}
	else
	{
		ui->tableWidget->setColumnHidden( ColumnBaseValue, true );// base value
		ui->tableWidget->setColumnHidden( ColumnInSizes, true );// in sizes
		ui->tableWidget->setColumnHidden( ColumnInHeights, true );// in heights
	}

	connect(ui->tableWidget, &QTableWidget::itemSelectionChanged, this, &TMainWindow::ShowMData);

	ShowUnits();

	ui->tableWidget->resizeColumnsToContents();
	ui->tableWidget->resizeRowsToContents();
	ui->tableWidget->horizontalHeader()->setStretchLastSection(true);
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::ShowUnits()
{
	const QString unit = UnitsToStr(mUnit);

	ShowHeaderUnits(ui->tableWidget, ColumnCalcValue, UnitsToStr(pUnit));// calculated value
	ShowHeaderUnits(ui->tableWidget, ColumnFormula, unit);// formula
	ShowHeaderUnits(ui->tableWidget, ColumnBaseValue, unit);// base value
	ShowHeaderUnits(ui->tableWidget, ColumnInSizes, unit);// in sizes
	ShowHeaderUnits(ui->tableWidget, ColumnInHeights, unit);// in heights
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::ShowHeaderUnits(QTableWidget *table, int column, const QString &unit)
{
	SCASSERT(table != nullptr)

	QString header = table->horizontalHeaderItem(column)->text();
	const int index = header.indexOf(QLatin1String("("));
	if (index != -1)
	{
		header.remove(index-1, 100);
	}
	const QString unitHeader = QString("%1 (%2)").arg(header, unit);
	table->horizontalHeaderItem(column)->setText(unitHeader);
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::MeasurementsWasSaved(bool saved)
{
	setWindowModified(!saved);
	!m_isReadOnly ? ui->actionSave->setEnabled(!saved): ui->actionSave->setEnabled(false);
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::SetCurrentFile(const QString &fileName)
{
	curFile = fileName;
	if (curFile.isEmpty())
	{
		ui->lineEditPathToFile->setText(QLatin1String("<") + tr("Empty") + QLatin1String(">"));
		ui->lineEditPathToFile->setToolTip(tr("File was not saved yet."));
		ui->lineEditPathToFile->setCursorPosition(0);
		ui->pushButtonShowInExplorer->setEnabled(false);
	}
	else
	{
		ui->lineEditPathToFile->setText(QDir::toNativeSeparators(curFile));
		ui->lineEditPathToFile->setToolTip(QDir::toNativeSeparators(curFile));
		ui->lineEditPathToFile->setCursorPosition(0);
		ui->pushButtonShowInExplorer->setEnabled(true);
		auto settings = qApp->seamlyMeSettings();
		QStringList files = settings->GetRecentFileList();
		files.removeAll(fileName);
		files.prepend(fileName);
		while (files.size() > MaxRecentFiles)
		{
			files.removeLast();
		}
		settings->SetRecentFileList(files);
		UpdateRecentFileActions();
	}

	UpdateWindowTitle();
}

//---------------------------------------------------------------------------------------------------------------------
bool TMainWindow::SaveMeasurements(const QString &fileName, QString &error)
{
	const bool result = individualMeasurements->SaveDocument(fileName, error);
	if (result)
	{
		SetCurrentFile(fileName);
		MeasurementsWasSaved(result);
	}
	return result;
}

//---------------------------------------------------------------------------------------------------------------------
bool TMainWindow::MaybeSave()
{
	if (this->isWindowModified())
	{
		if (curFile.isEmpty() && ui->tableWidget->rowCount() == 0)
		{
			return true;// Don't ask if file was created without modifications.
		}

		QScopedPointer<QMessageBox> messageBox(new QMessageBox(tr("Unsaved changes"),
															   tr("Measurements have been modified.\n"
																  "Do you want to save your changes?"),
															   QMessageBox::Warning, QMessageBox::Yes, QMessageBox::No,
															   QMessageBox::Cancel, this, Qt::Sheet));

		messageBox->setDefaultButton(QMessageBox::Yes);
		messageBox->setEscapeButton(QMessageBox::Cancel);

		messageBox->setButtonText(QMessageBox::Yes, curFile.isEmpty() || m_isReadOnly ? tr("Save...") : tr("Save"));
		messageBox->setButtonText(QMessageBox::No, tr("Don't Save"));

		messageBox->setWindowModality(Qt::ApplicationModal);
        messageBox->setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint
                                                 & ~Qt::WindowMaximizeButtonHint
                                                 & ~Qt::WindowMinimizeButtonHint);
		const QMessageBox::StandardButton ret = static_cast<QMessageBox::StandardButton>(messageBox->exec());

		switch (ret)
		{
			case QMessageBox::Yes:
				if (m_isReadOnly)
				{
					return FileSaveAs();
				}
				else
				{
					return FileSave();
				}
			case QMessageBox::No:
				return true;
			case QMessageBox::Cancel:
				return false;
			default:
				break;
		}
	}
	return true;
}

//---------------------------------------------------------------------------------------------------------------------
QTableWidgetItem *TMainWindow::AddCell(const QString &text, int row, int column, int aligment, bool ok)
{
	QTableWidgetItem *item = new QTableWidgetItem(text);
	item->setTextAlignment(aligment);
	item->setToolTip(text);

	// set the item non-editable (view only), and non-selectable
	Qt::ItemFlags flags = item->flags();
	flags &= ~(Qt::ItemIsEditable); // reset/clear the flag
	item->setFlags(flags);

	if (!ok)
	{
		QBrush brush = item->foreground();
		brush.setColor(Qt::red);
		item->setForeground(brush);
	}

	ui->tableWidget->setItem(row, column, item);

	return item;
}

//---------------------------------------------------------------------------------------------------------------------
QComboBox *TMainWindow::SetGradationList(QLabel *label, const QStringList &list)
{
	ui->toolBarGradation->addWidget(label);

	QComboBox *comboBox = new QComboBox;
	comboBox->addItems(list);
	ui->toolBarGradation->addWidget(comboBox);

	return comboBox;
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::SetDefaultHeight(int value)
{
	const qint32 index = gradationHeights->findText(QString("%1").arg(value));
	if (index != -1)
	{
		gradationHeights->setCurrentIndex(index);
	}
	else
	{
		currentHeight = gradationHeights->currentText().toInt();
	}
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::SetDefaultSize(int value)
{
	const qint32 index = gradationSizes->findText(QString("%1").arg(value));
	if (index != -1)
	{
		gradationSizes->setCurrentIndex(index);
	}
	else
	{
		currentSize = gradationSizes->currentText().toInt();
	}
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::RefreshData(bool freshCall)
{
	VContainer::ClearUniqueNames();
	data->ClearVariables(VarType::Measurement);
	individualMeasurements->readMeasurements();

	RefreshTable(freshCall);
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::RefreshTable(bool freshCall)
{
	ui->tableWidget->blockSignals(true);
	ui->tableWidget->clearContents();

	ShowUnits();

	const QMap<QString, QSharedPointer<MeasurementVariable> > table = data->DataMeasurements();
	QMap<int, QSharedPointer<MeasurementVariable> > orderedTable;
	QMap<QString, QSharedPointer<MeasurementVariable> >::const_iterator iterMap;
	for (iterMap = table.constBegin(); iterMap != table.constEnd(); ++iterMap)
	{
		QSharedPointer<MeasurementVariable> meash = iterMap.value();
		orderedTable.insert(meash->Index(), meash);
	}

	qint32 currentRow = -1;
	QMap<int, QSharedPointer<MeasurementVariable> >::const_iterator iMap;
	ui->tableWidget->setRowCount ( orderedTable.size() );
	for (iMap = orderedTable.constBegin(); iMap != orderedTable.constEnd(); ++iMap)
	{
		QSharedPointer<MeasurementVariable> meash = iMap.value();
		currentRow++;

		if (mType == MeasurementsType::Individual)
		{
			QTableWidgetItem *item = AddCell(qApp->translateVariables()->MToUser(meash->GetName()), currentRow, ColumnName,
											 Qt::AlignVCenter); // name
			item->setData(Qt::UserRole, meash->GetName());

			if (meash->isCustom())
			{
                AddCell(QStringLiteral("na"), currentRow, ColumnNumber, Qt::AlignVCenter);
				AddCell(meash->getGuiText(), currentRow, ColumnFullName, Qt::AlignVCenter);
			}
			else
			{

				AddCell(getMeasurementNumber(meash->GetName()), currentRow, ColumnNumber, Qt::AlignVCenter);
                AddCell(qApp->translateVariables()->guiText(meash->GetName()), currentRow, ColumnFullName, Qt::AlignVCenter);
			}

			const qreal value = UnitConvertor(*meash->GetValue(), mUnit, pUnit);
			AddCell(locale().toString(value), currentRow, ColumnCalcValue, Qt::AlignHCenter | Qt::AlignVCenter,
					meash->IsFormulaOk()); // calculated value

			QString formula;
			try
			{
				formula = qApp->translateVariables()->FormulaToUser(meash->GetFormula(), qApp->Settings()->getOsSeparator());
			}
			catch (qmu::QmuParserError &error)
			{
				Q_UNUSED(error)
				formula = meash->GetFormula();
			}

			AddCell(formula, currentRow, ColumnFormula, Qt::AlignVCenter); // formula
		}
		else
		{
			QTableWidgetItem *item = AddCell(qApp->translateVariables()->MToUser(meash->GetName()), currentRow, 0,
											 Qt::AlignVCenter); // name
			item->setData(Qt::UserRole, meash->GetName());

			if (meash->isCustom())
			{
				AddCell(QStringLiteral("na"), currentRow, ColumnNumber, Qt::AlignVCenter);
                AddCell(meash->getGuiText(), currentRow, ColumnFullName, Qt::AlignVCenter);
			}
			else
			{
				AddCell(getMeasurementNumber(meash->GetName()), currentRow, ColumnNumber, Qt::AlignVCenter);
                AddCell(qApp->translateVariables()->guiText(meash->GetName()), currentRow, ColumnFullName, Qt::AlignVCenter);
			}

			const qreal value = UnitConvertor(*data->DataVariables()->value(meash->GetName())->GetValue(), mUnit,
											  pUnit);
			AddCell(locale().toString(value), currentRow, ColumnCalcValue,
					Qt::AlignHCenter | Qt::AlignVCenter, meash->IsFormulaOk()); // calculated value

			AddCell(locale().toString(meash->GetBase()), currentRow, ColumnBaseValue,
					Qt::AlignHCenter | Qt::AlignVCenter); // base value

			AddCell(locale().toString(meash->GetKsize()), currentRow, ColumnInSizes,
					Qt::AlignHCenter | Qt::AlignVCenter); // in sizes

			AddCell(locale().toString(meash->GetKheight()), currentRow, ColumnInHeights,
					Qt::AlignHCenter | Qt::AlignVCenter); // in heights
		}
	}

	if (freshCall)
	{
		ui->tableWidget->resizeColumnsToContents();
		ui->tableWidget->resizeRowsToContents();
	}
	ui->tableWidget->horizontalHeader()->setStretchLastSection(true);
	ui->tableWidget->blockSignals(false);

	if (ui->tableWidget->rowCount() > 0)
	{
		ui->actionExportToCSV->setEnabled(true);
	}
}

//---------------------------------------------------------------------------------------------------------------------
QString TMainWindow::GetCustomName() const
{
	qint32 num = 1;
	QString name;
	do
	{
		name = CustomMSign + qApp->translateVariables()->InternalVarToUser(measurement_) + QString().number(num);
		num++;
	} while (data->IsUnique(name) == false);

	return name;
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::Controls()
{
	if (ui->tableWidget->rowCount() > 0)
	{
		ui->toolButtonRemove->setEnabled(true);
	}
	else
	{
		ui->toolButtonRemove->setEnabled(false);
	}

	if (ui->tableWidget->rowCount() >= 2)
	{
		if (ui->tableWidget->currentRow() == 0)
		{
			ui->toolButtonTop->setEnabled(false);
			ui->toolButtonUp->setEnabled(false);
			ui->toolButtonDown->setEnabled(true);
			ui->toolButtonBottom->setEnabled(true);
		}
		else if (ui->tableWidget->currentRow() == ui->tableWidget->rowCount()-1)
		{
			ui->toolButtonTop->setEnabled(true);
			ui->toolButtonUp->setEnabled(true);
			ui->toolButtonDown->setEnabled(false);
			ui->toolButtonBottom->setEnabled(false);
		}
		else
		{
			ui->toolButtonTop->setEnabled(true);
			ui->toolButtonUp->setEnabled(true);
			ui->toolButtonDown->setEnabled(true);
			ui->toolButtonBottom->setEnabled(true);
		}
	}
	else
	{
		ui->toolButtonTop->setEnabled(false);
		ui->toolButtonUp->setEnabled(false);
		ui->toolButtonDown->setEnabled(false);
		ui->toolButtonBottom->setEnabled(false);
	}
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::MFields(bool enabled)
{
	ui->lineEditName->setEnabled(enabled);
	ui->plainTextEditDescription->setEnabled(enabled);
	ui->lineEditFullName->setEnabled(enabled);

	if (mType == MeasurementsType::Multisize)
	{
		ui->doubleSpinBoxBaseValue->setEnabled(enabled);
		ui->doubleSpinBoxInSizes->setEnabled(enabled);
		ui->doubleSpinBoxInHeights->setEnabled(enabled);
	}
	else
	{
		ui->plainTextEditFormula->setEnabled(enabled);
		ui->toolButtonExpr->setEnabled(enabled);
	}

	ui->lineEditFind->setEnabled(enabled);
	if (enabled && !ui->lineEditFind->text().isEmpty())
	{
		ui->toolButtonFindPrevious->setEnabled(enabled);
		ui->toolButtonFindNext->setEnabled(enabled);
	}
	else
	{
		ui->toolButtonFindPrevious->setEnabled(false);
		ui->toolButtonFindNext->setEnabled(false);
	}
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::UpdateWindowTitle()
{
	QString fileName;
	bool isFileWritable = true;
	if (!curFile.isEmpty())
	{
#ifdef Q_OS_WIN32
		qt_ntfs_permission_lookup++; // turn checking on
#endif /*Q_OS_WIN32*/
		isFileWritable = QFileInfo(curFile).isWritable();
#ifdef Q_OS_WIN32
		qt_ntfs_permission_lookup--; // turn it off again
#endif /*Q_OS_WIN32*/
		fileName = curFile;
	}
	else
	{
		fileName = tr("untitled %1").arg(qApp->mainWindows().size() + 1);
		mType == MeasurementsType::Multisize ? fileName += QLatin1String(".") + smmsExt :
                                               fileName += QLatin1String(".") + smisExt;
	}

	fileName += QLatin1String("[*]");

	if (m_isReadOnly || !isFileWritable)
	{
		fileName += QLatin1String(" (") + tr("read only") + QLatin1String(")");
	}

	setWindowTitle( VER_INTERNALNAME_ME_STR + QString(" - ") + fileName);
	setWindowFilePath(curFile);

#if defined(Q_OS_MAC)
	static QIcon fileIcon = QIcon(QCoreApplication::applicationDirPath() +
								  QLatin1String("/../Resources/measurements.icns"));
	QIcon icon;
	if (!curFile.isEmpty())
	{
		if (!isWindowModified())
		{
			icon = fileIcon;
		}
		else
		{
			static QIcon darkIcon;

			if (darkIcon.isNull())
			{
				darkIcon = QIcon(darkenPixmap(fileIcon.pixmap(16, 16)));
			}
			icon = darkIcon;
		}
	}
	setWindowIcon(icon);
#endif //defined(Q_OS_MAC)
}

//---------------------------------------------------------------------------------------------------------------------
QString TMainWindow::ClearCustomName(const QString &name) const
{
	QString clear = name;
	const int index = clear.indexOf(CustomMSign);
	if (index == 0)
	{
		clear.remove(0, 1);
	}
	return clear;
}

//---------------------------------------------------------------------------------------------------------------------
bool TMainWindow::EvalFormula(const QString &formula, bool fromUser, VContainer *data, QLabel *label)
{
	const QString postfix = UnitsToStr(pUnit);//Show unit in dialog label (cm, mm or inch)
	if (formula.isEmpty())
	{
		label->setText(tr("Error") + " (" + postfix + "). " + tr("Empty field."));
		label->setToolTip(tr("Empty field"));
		return false;
	}
	else
	{
		try
		{
			// Replace line return character with spaces for calc if exist
			QString f;
			if (fromUser)
			{
				f = qApp->translateVariables()->FormulaFromUser(formula, qApp->Settings()->getOsSeparator());
			}
			else
			{
				f = formula;
			}
			f.replace("\n", " ");
			QScopedPointer<Calculator> cal(new Calculator());
			qreal result = cal->EvalFormula(data->DataVariables(), f);

			if (qIsInf(result) || qIsNaN(result))
			{
				label->setText(tr("Error") + " (" + postfix + ").");
				label->setToolTip(tr("Invalid result. Value is infinite or NaN. Please, check your calculations."));
				return false;
			}

			result = UnitConvertor(result, mUnit, pUnit);

			label->setText(qApp->LocaleToString(result) + " " +postfix);
			label->setToolTip(tr("Value"));
			return true;
		}
		catch (qmu::QmuParserError &error)
		{
			label->setText(tr("Error") + " (" + postfix + "). " + tr("Parser error: %1").arg(error.GetMsg()));
			label->setToolTip(tr("Parser error: %1").arg(error.GetMsg()));
			return false;
		}
	}
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::Open(const QString &dir, const QString &filter)
{
    const QString filename = fileDialog(this, tr("Open file"), dir, filter, nullptr, QFileDialog::DontUseNativeDialog,
                                        QFileDialog::ExistingFile, QFileDialog::AcceptOpen);

	if (!filename.isEmpty())
	{
		if (individualMeasurements == nullptr)
		{
			LoadFile(filename);
		}
		else
		{
			qApp->newMainWindow()->LoadFile(filename);
		}
	}
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::UpdatePadlock(bool ro)
{
	ui->actionReadOnly->setChecked(ro);
	if (ro)
	{
		ui->actionReadOnly->setIcon(QIcon("://seamlymeicon/24x24/padlock_locked.png"));
	}
	else
	{
		ui->actionReadOnly->setIcon(QIcon("://seamlymeicon/24x24/padlock_opened.png"));
	}

	ui->actionReadOnly->setDisabled(m_isReadOnly);
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::MeasurementGUI()
{
	if (const QTableWidgetItem *nameField = ui->tableWidget->item(ui->tableWidget->currentRow(), ColumnName))
	{
		const bool isCustom = !(nameField->text().indexOf(CustomMSign) == 0);
		ui->lineEditName->setReadOnly(isCustom);
		ui->plainTextEditDescription->setReadOnly(isCustom);
		ui->lineEditFullName->setReadOnly(isCustom);

		// Need to block signals for QLineEdit in readonly mode because it still emits
		// QLineEdit::editingFinished signal.
		ui->lineEditName->blockSignals(isCustom);
		ui->lineEditFullName->blockSignals(isCustom);

		Controls(); // Buttons remove, up, down
	}
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::ReadSettings()
{
	const VSeamlyMeSettings *settings = qApp->seamlyMeSettings();
	restoreGeometry(settings->GetGeometry());
	restoreState(settings->GetWindowState());
	restoreState(settings->GetToolbarsState(), APP_VERSION);

	// Text under tool button icon
	ToolBarStyles();

	// Stack limit
	//qApp->getUndoStack()->setUndoLimit(settings->GetUndoCount());
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::WriteSettings()
{
	VSeamlyMeSettings *settings = qApp->seamlyMeSettings();
	settings->SetGeometry(saveGeometry());
	settings->SetWindowState(saveState());
	settings->SetToolbarsState(saveState(APP_VERSION));
}

//---------------------------------------------------------------------------------------------------------------------
QStringList TMainWindow::FilterMeasurements(const QStringList &mNew, const QStringList &mFilter)
{
    return convertToList(convertToSet<QString>(mNew).subtract(convertToSet<QString>(mFilter)));
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::UpdatePatternUnit()
{
	const int row = ui->tableWidget->currentRow();

	if (row == -1)
	{
		return;
	}

	RefreshTable();

	search->RefreshList(ui->lineEditFind->text());

	ui->tableWidget->selectRow(row);
}

//---------------------------------------------------------------------------------------------------------------------
bool TMainWindow::LoadFromExistingFile(const QString &path)
{
	if (individualMeasurements == nullptr)
	{
		if (!QFileInfo(path).exists())
		{
			qCCritical(tMainWindow, "%s", qUtf8Printable(tr("File '%1' doesn't exist!").arg(path)));
			if (qApp->isTestMode())
			{
				qApp->exit(V_EX_NOINPUT);
			}
			return false;
		}

		// Check if file already opened
		QList<TMainWindow*>list = qApp->mainWindows();
		for (int i = 0; i < list.size(); ++i)
		{
			if (list.at(i)->CurrentFile() == path)
			{
				list.at(i)->activateWindow();
				close();
				return false;
			}
		}

		VlpCreateLock(lock, path);

		if (!lock->IsLocked())
		{
			if (!IgnoreLocking(lock->GetLockError(), path))
			{
				return false;
			}
		}

		try
		{
			data = new VContainer(qApp->translateVariables(), &mUnit);

			individualMeasurements = new MeasurementDoc(data);
			individualMeasurements->setSize(&currentSize);
			individualMeasurements->setHeight(&currentHeight);
			individualMeasurements->setXMLContent(path);

			mType = individualMeasurements->Type();

			if (mType == MeasurementsType::Unknown)
			{
				VException e(tr("File has unknown format."));
				throw e;
			}

			if (mType == MeasurementsType::Multisize)
			{
				VException e(tr("Export from multisize measurements is not supported."));
				throw e;
			}
			else
			{
				IndividualSizeConverter converter(path);
				m_curFileFormatVersion = converter.getCurrentFormatVersion();
				m_curFileFormatVersionStr = converter.getVersionStr();
				individualMeasurements->setXMLContent(converter.Convert());// Read again after conversion
			}

			if (!individualMeasurements->eachKnownNameIsValid())
			{
				VException e(tr("File contains invalid known measurement(s)."));
				throw e;
			}

			mUnit = individualMeasurements->measurementUnits();
			pUnit = mUnit;

			currentHeight = individualMeasurements->BaseHeight();
			currentSize = individualMeasurements->BaseSize();

			ui->labelToolTip->setVisible(false);
			ui->tabWidget->setVisible(true);

			InitWindow();

			individualMeasurements->ClearForExport();
			const bool freshCall = true;
			RefreshData(freshCall);

			if (ui->tableWidget->rowCount() > 0)
			{
				ui->tableWidget->selectRow(0);
			}

			lock.reset();// Now we can unlock the file

			m_isReadOnly = individualMeasurements->isReadOnly();
			UpdatePadlock(m_isReadOnly);
			MeasurementGUI();
		}

		catch (VException &exception)
		{
			qCCritical(tMainWindow, "%s\n\n%s\n\n%s", qUtf8Printable(tr("File error.")),
					   qUtf8Printable(exception.ErrorMessage()), qUtf8Printable(exception.DetailedInformation()));

			ui->labelToolTip->setVisible(true);
			ui->tabWidget->setVisible(false);
			delete individualMeasurements;
			individualMeasurements = nullptr;
			delete data;
			data = nullptr;
			lock.reset();

			if (qApp->isTestMode())
			{
				qApp->exit(V_EX_NOINPUT);
			}
			return false;
		}
	}
	else
	{
		qApp->newMainWindow();
		return qApp->mainWindow()->LoadFile(path);
	}

	return true;
}
//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::UpdateRecentFileActions()
{
	qCDebug(tMainWindow, "Updating recent file actions.");
	const QStringList files = qApp->seamlyMeSettings()->GetRecentFileList();
	const int numRecentFiles = qMin(files.size(), static_cast<int>(MaxRecentFiles));
	qCDebug(tMainWindow, "Updating recent file actions = %i ",numRecentFiles);

	for (int i = 0; i < numRecentFiles; ++i)
	{
		const QString text = QString("&%1. %2").arg(i + 1).arg(strippedName(files.at(i)));
		qCDebug(tMainWindow, "file %i = %s", numRecentFiles, qUtf8Printable(text));
		recentFileActs[i]->setText(text);
		recentFileActs[i]->setData(files.at(i));
		recentFileActs[i]->setVisible(true);
	}

	for (int j = numRecentFiles; j < MaxRecentFiles; ++j)
	{
		recentFileActs[j]->setVisible(false);
	}

	separatorAct->setVisible(numRecentFiles>0);
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::CreateWindowMenu(QMenu *menu)
{
	SCASSERT(menu != nullptr)

	QAction *action = menu->addAction(tr("&New Window"));
	connect(action, &QAction::triggered, this, []()
	{
		qApp->newMainWindow();
		qApp->mainWindow()->activateWindow();
	});
	action->setMenuRole(QAction::NoRole);
	menu->addSeparator();

	const QList<TMainWindow*> windows = qApp->mainWindows();
	for (int i = 0; i < windows.count(); ++i)
	{
		TMainWindow *window = windows.at(i);

		QString title = QString("%1. %2").arg(i+1).arg(window->windowTitle());
		const int index = title.lastIndexOf("[*]");
		if (index != -1)
		{
			window->isWindowModified() ? title.replace(index, 3, "*") : title.replace(index, 3, "");
		}

		QAction *action = menu->addAction(title, this, SLOT(ShowWindow()));
		action->setData(i);
		action->setCheckable(true);
		action->setMenuRole(QAction::NoRole);
		if (window->isActiveWindow())
		{
			action->setChecked(true);
		}
	}
}

//---------------------------------------------------------------------------------------------------------------------
bool TMainWindow::IgnoreLocking(int error, const QString &path)
{
	QMessageBox::StandardButton answer = QMessageBox::Abort;
	if (!qApp->isTestMode())
	{
		switch(error)
		{
			case QLockFile::LockFailedError:
				answer = QMessageBox::warning(this, tr("Locking file"),
											  tr("This file already opened in another window. Ignore if you want "
												 "to continue (not recommended, can cause a data corruption)."),
											  QMessageBox::Abort|QMessageBox::Ignore, QMessageBox::Abort);
				break;
			case QLockFile::PermissionError:
				answer = QMessageBox::question(this, tr("Locking file"),
											   tr("The lock file could not be created, for lack of permissions. "
												  "Ignore if you want to continue (not recommended, can cause "
												  "a data corruption)."),
											   QMessageBox::Abort|QMessageBox::Ignore, QMessageBox::Abort);
				break;
			case QLockFile::UnknownError:
				answer = QMessageBox::question(this, tr("Locking file"),
											   tr("Unknown error happened, for instance a full partition "
												  "prevented writing out the lock file. Ignore if you want to "
												  "continue (not recommended, can cause a data corruption)."),
											   QMessageBox::Abort|QMessageBox::Ignore, QMessageBox::Abort);
				break;
			default:
				answer = QMessageBox::Abort;
				break;
		}
	}

	if (answer == QMessageBox::Abort)
	{
		qCWarning(tMainWindow, "Failed to lock %s", qUtf8Printable(path));
		qCWarning(tMainWindow, "Error type: %d", error);
		if (qApp->isTestMode())
		{
			switch(error)
			{
				case QLockFile::LockFailedError:
					qCCritical(tMainWindow, "%s",
							   qUtf8Printable(tr("This file already opened in another window.")));
					break;
				case QLockFile::PermissionError:
					qCCritical(tMainWindow, "%s",
							   qUtf8Printable(tr("The lock file could not be created, for lack of permissions.")));
					break;
				case QLockFile::UnknownError:
					qCCritical(tMainWindow, "%s",
							   qUtf8Printable(tr("Unknown error happened, for instance a full partition "
												 "prevented writing out the lock file.")));
					break;
				default:
					break;
			}

			qApp->exit(V_EX_NOINPUT);
		}
		return false;
	}
	return true;
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::SetDecimals()
{
	switch (mUnit)
	{
		case Unit::Cm:
			ui->doubleSpinBoxBaseValue->setDecimals(2);
			ui->doubleSpinBoxBaseValue->setSingleStep(0.01);

			ui->doubleSpinBoxInSizes->setDecimals(2);
			ui->doubleSpinBoxInSizes->setSingleStep(0.01);

			ui->doubleSpinBoxInHeights->setDecimals(2);
			ui->doubleSpinBoxInHeights->setSingleStep(0.01);
			break;
		case Unit::Mm:
			ui->doubleSpinBoxBaseValue->setDecimals(1);
			ui->doubleSpinBoxBaseValue->setSingleStep(0.1);

			ui->doubleSpinBoxInSizes->setDecimals(1);
			ui->doubleSpinBoxInSizes->setSingleStep(0.1);

			ui->doubleSpinBoxInHeights->setDecimals(1);
			ui->doubleSpinBoxInHeights->setSingleStep(0.1);
			break;
		case Unit::Inch:
			ui->doubleSpinBoxBaseValue->setDecimals(5);
			ui->doubleSpinBoxBaseValue->setSingleStep(0.00001);

			ui->doubleSpinBoxInSizes->setDecimals(5);
			ui->doubleSpinBoxInSizes->setSingleStep(0.00001);

			ui->doubleSpinBoxInHeights->setDecimals(5);
			ui->doubleSpinBoxInHeights->setSingleStep(0.00001);
			break;
		default:
			break;
	}
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::initUnits()
{
	labelPatternUnit = new QLabel(tr("Pattern unit:"));
	ui->toolBarGradation->addWidget(labelPatternUnit);

	comboBoxUnits = new QComboBox(this);
	InitComboBoxUnits();

	// set default unit
	const qint32 indexUnit = comboBoxUnits->findData(static_cast<int>(pUnit));
	if (indexUnit != -1)
	{
		comboBoxUnits->setCurrentIndex(indexUnit);
	}

	connect(comboBoxUnits, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this,
			&TMainWindow::PatternUnitChanged);

	ui->toolBarGradation->addWidget(comboBoxUnits);
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::InitComboBoxUnits()
{
	SCASSERT(comboBoxUnits != nullptr)
	comboBoxUnits->addItem(UnitsToStr(Unit::Cm, true), QVariant(static_cast<int>(Unit::Cm)));
	comboBoxUnits->addItem(UnitsToStr(Unit::Mm, true), QVariant(static_cast<int>(Unit::Mm)));
	comboBoxUnits->addItem(UnitsToStr(Unit::Inch, true), QVariant(static_cast<int>(Unit::Inch)));
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::InitGender(QComboBox *gender)
{
	SCASSERT(gender != nullptr)
	gender->addItem(tr("unknown", "gender"), QVariant(static_cast<int>(GenderType::Unknown)));
	gender->addItem(tr("male", "gender"), QVariant(static_cast<int>(GenderType::Male)));
	gender->addItem(tr("female", "gender"), QVariant(static_cast<int>(GenderType::Female)));
}

//---------------------------------------------------------------------------------------------------------------------
template <class T>
void TMainWindow::HackWidget(T **widget)
{
	delete *widget;
	*widget = new T();
	hackedWidgets.append(*widget);
}

//---------------------------------------------------------------------------------------------------------------------
void TMainWindow::zoomToSelected()
{
    // do nothing
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief copyToClipboard copy dialog selection to clipboard as comma separated values.
 */
void TMainWindow::copyToClipboard()
{
    QItemSelectionModel *model = ui->tableWidget->selectionModel();
    QModelIndexList selectedIndexes = model->selectedIndexes();

    QString clipboardString;

    for (int i = 0; i < selectedIndexes.count(); ++i)
    {
        QModelIndex current = selectedIndexes[i];
        QString displayText = current.data(Qt::DisplayRole).toString();

        // Check if another column exists beyond this one.
        if (i + 1 < selectedIndexes.count())
        {
            QModelIndex next = selectedIndexes[i+1];

            // If the column is on different row, the clipboard should take note.
            if (next.row() != current.row())
            {
                displayText.append("\n");
            }
            else
            {
                // Otherwise append a comma separator.
                displayText.append(" , ");
            }
        }
        clipboardString.append(displayText);
    }

    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(clipboardString);
}
