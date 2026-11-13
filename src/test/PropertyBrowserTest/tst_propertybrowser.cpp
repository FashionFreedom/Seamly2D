// SPDX-License-Identifier: GPL-3.0-or-later

#include "core/application_2d.h"
#include "core/vtooloptionspropertybrowser.h"
#include "mainwindow.h"
#include "xml/vpattern.h"
#include "../vtools/tools/drawTools/toolcurve/vtoolarc.h"
#include "../vtools/tools/drawTools/toolcurve/vtoolcubicbezier.h"
#include "../vtools/tools/drawTools/toolpoint/toolsinglepoint/basepoint_tool.h"
#include "../vtools/tools/drawTools/toolpoint/toolsinglepoint/toollinepoint/vtoolendline.h"
#include "../vtools/tools/drawTools/vtoolline.h"
#include "../vpatterndb/vformula.h"

#include <QDockWidget>
#include <QFile>
#include <QGraphicsSceneHoverEvent>
#include <QLineEdit>
#include <QStandardPaths>
#include <QTemporaryDir>
#include <QTranslator>
#include <QUndoStack>
#include <QtTest>
#include <memory>

class TST_PropertyBrowser : public QObject
{
    Q_OBJECT

private slots:
    void cleanup();
    void lineGeometry_data();
    void lineGeometry();
    void editsAndUndoRedo();
    void otherToolProperties();
    void saveAndReopen();
    void spanishLabels();

private:
    QTemporaryDir m_directory;
    std::unique_ptr<MainWindow> m_window;
    VToolOptionsPropertyBrowser *m_browser = nullptr;
    VPE::VPropertyModel *m_model = nullptr;
    VToolLine *m_line = nullptr;
    VPattern *m_document = nullptr;

    bool load(const QString &unit = QStringLiteral("cm"), double angle = 0, bool zero = false);
    bool open(const QString &path);
    QString value(const QString &id) const;
    qreal formulaValue(const QString &id) const;
};

//---------------------------------------------------------------------------------------------------------------------
bool TST_PropertyBrowser::load(const QString &unit, double angle, bool zero)
{
    QFile source(QFINDTESTDATA("share/line.sm2d"));
    if (!m_directory.isValid() || !source.open(QIODevice::ReadOnly))
    {
        return false;
    }

    QDomDocument xml;
    if (!xml.setContent(source.readAll()))
    {
        return false;
    }
    xml.documentElement().firstChildElement("unit").firstChild().setNodeValue(unit);
    auto calculation = xml.documentElement().firstChildElement("draftBlock").firstChildElement("calculation");
    for (auto element = calculation.firstChildElement(); !element.isNull(); element = element.nextSiblingElement())
    {
        if (element.attribute("id") == "2")
        {
            element.setAttribute("angle", QString::number(angle, 'g', 15));
        }
        else if (element.attribute("id") == "5" && zero)
        {
            element.setAttribute("secondPoint", "1");
        }
    }

    const QString path = m_directory.filePath("line.sm2d");
    QFile fixture(path);
    if (!fixture.open(QIODevice::WriteOnly | QIODevice::Truncate) || fixture.write(xml.toByteArray()) < 0)
    {
        return false;
    }
    fixture.close();
    return open(path);
}

//---------------------------------------------------------------------------------------------------------------------
bool TST_PropertyBrowser::open(const QString &path)
{
    m_window.reset(new MainWindow);
    qApp->setMainWindow(m_window.get());
    if (!m_window->LoadPattern(path))
    {
        return false;
    }
    m_document = qobject_cast<VPattern *>(qApp->getCurrentDocument());
    m_browser = m_window->findChild<VToolOptionsPropertyBrowser *>();
    m_line = dynamic_cast<VToolLine *>(VAbstractPattern::getTool(5));
    if (!m_document || !m_browser || !m_line)
    {
        return false;
    }
    m_model = m_browser->findChild<VPE::VPropertyModel *>();
    if (!m_model)
    {
        return false;
    }
    m_browser->itemClicked(m_line);
    return true;
}

//---------------------------------------------------------------------------------------------------------------------
QString TST_PropertyBrowser::value(const QString &id) const
{
    const auto property = m_model->getProperty(id);
    return property ? property->getValue().toString() : QString();
}

//---------------------------------------------------------------------------------------------------------------------
qreal TST_PropertyBrowser::formulaValue(const QString &id) const
{
    auto formula = m_model->getProperty(id)->getValue().value<VFormula>();
    formula.Eval();
    return formula.getDoubleValue();
}

//---------------------------------------------------------------------------------------------------------------------
void TST_PropertyBrowser::cleanup()
{
    if (m_browser)
    {
        m_browser->itemClicked(nullptr);
    }
    m_window.reset();
    qApp->setMainWindow(nullptr);
    qApp->getUndoStack()->clear();
    m_browser = nullptr;
    m_model = nullptr;
    m_document = nullptr;
    m_line = nullptr;
}

//---------------------------------------------------------------------------------------------------------------------
void TST_PropertyBrowser::lineGeometry_data()
{
    QTest::addColumn<QString>("unit");
    QTest::addColumn<double>("angle");
    QTest::addColumn<bool>("zero");
    for (const auto &unit : {"cm", "mm", "inch"})
    {
        for (double angle : {0., 90., 180., 270., 36.869897645844})
        {
            QTest::newRow(qPrintable(QString("%1-%2").arg(unit).arg(angle))) << QString(unit) << angle << false;
        }
        QTest::newRow(qPrintable(QString("%1-zero").arg(unit))) << QString(unit) << 0. << true;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void TST_PropertyBrowser::lineGeometry()
{
    QFETCH(QString, unit);
    QFETCH(double, angle);
    QFETCH(bool, zero);
    QVERIFY(load(unit, angle, zero));
    auto lengthProperty = m_model->getProperty(AttrLength);
    auto angleProperty = m_model->getProperty(AttrAngle);
    QVERIFY(lengthProperty);
    QVERIFY(angleProperty);
    QVERIFY(lengthProperty->getSetting("ReadOnly").toBool());
    QVERIFY(angleProperty->getSetting("ReadOnly").toBool());
    QCOMPARE(value(AttrLength), QString("%1 %2").arg(zero ? 0 : 10).arg(unit));
    QVERIFY(value(AttrAngle).endsWith(degreeSymbol));
    QVERIFY(qAbs(value(AttrAngle).chopped(1).toDouble() - (zero ? 0 : angle)) < 0.0001);

    // Compare with the actual hover tooltip, not a second implementation of the calculation.
    QGraphicsSceneHoverEvent hover(QEvent::GraphicsSceneHoverEnter);
    QVERIFY(m_line->scene()->sendEvent(m_line, &hover));
    QVERIFY(m_line->toolTip().contains(value(AttrLength)));
    QVERIFY(m_line->toolTip().contains(value(AttrAngle)));

    const auto dock = qobject_cast<QDockWidget *>(m_browser->parent());
    QVERIFY(dock);
    int readOnlyFields = 0;
    for (auto editor : dock->findChildren<QLineEdit *>())
    {
        if (editor->text() == value(AttrLength) || editor->text() == value(AttrAngle))
        {
            QVERIFY(editor->isReadOnly());
            const QString before = editor->text();
            QTest::keyClicks(editor, "123");
            QCOMPARE(editor->text(), before);
            ++readOnlyFields;
        }
    }
    QCOMPARE(readOnlyFields, 2);
    m_browser->itemClicked(nullptr);
    QCOMPARE(m_model->rowCount(), 0);
    m_browser->itemClicked(m_line);
    QCOMPARE(value(AttrLength), QString("%1 %2").arg(zero ? 0 : 10).arg(unit));
}

//---------------------------------------------------------------------------------------------------------------------
void TST_PropertyBrowser::editsAndUndoRedo()
{
    QVERIFY(load());
    auto point = dynamic_cast<VToolEndLine *>(VAbstractPattern::getTool(2));
    QVERIFY(point);
    point->SetFormulaLength(VFormula("12.5", point->getData()));
    QTRY_COMPARE(value(AttrLength), QString("12.5 cm"));
    qApp->getUndoStack()->undo();
    QTRY_COMPARE(value(AttrLength), QString("10 cm"));
    qApp->getUndoStack()->redo();
    QTRY_COMPARE(value(AttrLength), QString("12.5 cm"));

    m_line->SetSecondPoint(3);
    QTRY_COMPARE(value(AttrLength), QString("7 cm"));
    QTRY_COMPARE(value(AttrAngle), QString("270") + degreeSymbol);
    qApp->getUndoStack()->undo();
    QTRY_COMPARE(value(AttrLength), QString("12.5 cm"));
    QTRY_COMPARE(value(AttrAngle), QString("0") + degreeSymbol);
    qApp->getUndoStack()->redo();
    QTRY_COMPARE(value(AttrLength), QString("7 cm"));
    qApp->getUndoStack()->undo();
    QTRY_COMPARE(value(AttrLength), QString("12.5 cm"));

    point->SetFormulaAngle(VFormula("90", point->getData()));
    QTRY_COMPARE(value(AttrAngle), QString("90") + degreeSymbol);
    qApp->getUndoStack()->undo();
    QTRY_COMPARE(value(AttrAngle), QString("0") + degreeSymbol);

    // A queued refresh must be harmless if selection is cleared before delivery.
    point->SetFormulaLength(VFormula("11", point->getData()));
    m_browser->itemClicked(nullptr);
    QCoreApplication::processEvents();
    QCOMPARE(m_model->rowCount(), 0);
}

//---------------------------------------------------------------------------------------------------------------------
void TST_PropertyBrowser::otherToolProperties()
{
    QVERIFY(load());
    auto point = dynamic_cast<BasePointTool *>(VAbstractPattern::getTool(1));
    auto curve = dynamic_cast<VToolCubicBezier *>(VAbstractPattern::getTool(6));
    auto arc = dynamic_cast<VToolArc *>(VAbstractPattern::getTool(7));
    QVERIFY(point);
    QVERIFY(curve);
    QVERIFY(arc);

    m_browser->itemClicked(point);
    const QPointF before = point->GetBasePointPos();
    point->SetBasePointPos(before + QPointF(1, 2));
    QTRY_COMPARE(m_model->getProperty("Coordinates")->data(VPE::VProperty::DPC_Data, Qt::DisplayRole).toPointF(),
                 before + QPointF(1, 2));
    qApp->getUndoStack()->undo();
    QTRY_COMPARE(m_model->getProperty("Coordinates")->data(VPE::VProperty::DPC_Data, Qt::DisplayRole).toPointF(), before);

    m_browser->itemClicked(curve);
    curve->SetAutoSmooth(true);
    QTRY_COMPARE(m_model->getProperty(AttrAutoSmooth)->getValue().toInt(), 1);
    qApp->getUndoStack()->undo();
    QTRY_COMPARE(m_model->getProperty(AttrAutoSmooth)->getValue().toInt(), 0);

    m_browser->itemClicked(arc);
    arc->SetFormulaRadius(VFormula("5", arc->getData()));
    QTRY_COMPARE(formulaValue(AttrRadius), qreal(5));
    qApp->getUndoStack()->undo();
    QTRY_COMPARE(formulaValue(AttrRadius), qreal(3));
    m_browser->itemClicked(m_line);
    QCOMPARE(value(AttrLength), QString("10 cm"));
}

//---------------------------------------------------------------------------------------------------------------------
void TST_PropertyBrowser::saveAndReopen()
{
    QVERIFY(load());
    auto point = dynamic_cast<VToolEndLine *>(VAbstractPattern::getTool(2));
    QVERIFY(point);
    point->SetFormulaLength(VFormula("12.5", point->getData()));
    QTRY_COMPARE(value(AttrLength), QString("12.5 cm"));
    const QString path = m_directory.filePath("saved.sm2d");
    QString error;
    QVERIFY2(m_document->SaveDocument(path, error), qPrintable(error));
    cleanup();
    QVERIFY(open(path));
    QCOMPARE(value(AttrLength), QString("12.5 cm"));
    QCOMPARE(value(AttrAngle), QString("0") + degreeSymbol);
}

//---------------------------------------------------------------------------------------------------------------------
void TST_PropertyBrowser::spanishLabels()
{
    QTranslator spanish;
    QVERIFY(spanish.load(":/i18n/seamly2d_es_ES.qm"));
    qApp->installTranslator(&spanish);
    QVERIFY(load());
    QCOMPARE(m_model->getProperty(AttrLength)->getName(), QString::fromUtf8("Longitud:"));
    QCOMPARE(m_model->getProperty(AttrAngle)->getName(), QString::fromUtf8("Ángulo:"));
    qApp->removeTranslator(&spanish);
}

//---------------------------------------------------------------------------------------------------------------------
int main(int argc, char **argv)
{
    Q_INIT_RESOURCE(cursor);
    Q_INIT_RESOURCE(icon);
    Q_INIT_RESOURCE(schema);
    Q_INIT_RESOURCE(theme);
    Q_INIT_RESOURCE(flags);
    Q_INIT_RESOURCE(icons);
    Q_INIT_RESOURCE(toolicon);
    Q_INIT_RESOURCE(sounds);
    Q_INIT_RESOURCE(diagrams);

    // Isolate settings, backups and pattern files from the interactive user.
    QTemporaryDir dataDirectory;
    if (!dataDirectory.isValid())
    {
        return V_EX_CANTCREAT;
    }
    QStandardPaths::setTestModeEnabled(true);
    qputenv("SEAMLY_DATA_DIR", dataDirectory.path().toUtf8());
    const int status = VCommonSettings::configureDataDirectory();
    if (status != V_EX_OK)
    {
        return status;
    }

    // Let QtTest handle its arguments instead of the application's CLI parser.
    int applicationArgc = 1;
    char *applicationArgv[] = {argv[0], nullptr};
    Application2D app(applicationArgc, applicationArgv);
    app.Seamly2DSettings()->setShowWelcome(true);
    app.Seamly2DSettings()->setAutosaveState(false);
    app.Seamly2DSettings()->setMaxBackups(1000);
    const QString backups = dataDirectory.filePath("backups");
    QDir().mkpath(backups);
    app.Seamly2DSettings()->setBackupFilePath(backups);
    app.loadTranslations("en_US");
    TST_PropertyBrowser test;
    return QTest::qExec(&test, argc, argv);
}

#include "tst_propertybrowser.moc"
