// SPDX-License-Identifier: GPL-3.0-or-later
#include "window.h"
#include "dressform.h"
#include <QApplication>
#include <QCommandLineParser>
#include <QTextStream>
#include <QTimer>
#include <QElapsedTimer>
#include <QQuickWindow>
#include <QSGRendererInterface>
#include <memory>

int main(int argc, char **argv)
{
    // Validation is a CPU operation, including in a deployed GUI app bundle.
    // Respect the option terminator and values of our options while deciding
    // whether a platform plugin is needed; the parser below validates all input.
    bool cpuOnly=false;
    for (int i=1; i<argc; ++i)
    {
        const QByteArray argument(argv[i]);
        if (argument=="--") break;
        if (argument=="--render" || argument=="--steps") { ++i; continue; }
        if (argument=="--validate") cpuOnly=true;
    }
#ifdef Q_OS_MAC
    if (!cpuOnly && qEnvironmentVariableIsEmpty("QSG_RHI_BACKEND"))
        QQuickWindow::setGraphicsApi(QSGRendererInterface::Metal);
#endif
    std::unique_ptr<QCoreApplication> app;
    if (cpuOnly) app=std::make_unique<QCoreApplication>(argc,argv);
    else app=std::make_unique<QApplication>(argc,argv);
    QCoreApplication::setApplicationName(QStringLiteral("Seamly3D"));
    QCoreApplication::setApplicationVersion(QStringLiteral("0.1"));
    QCommandLineParser parser; parser.setApplicationDescription(QStringLiteral("Experimental sewing assembly preview"));
    parser.addHelpOption(); parser.addVersionOption();
    parser.addPositionalArgument(QStringLiteral("project"),QStringLiteral("Project exported from Seamly2D (.sm3d)"),QStringLiteral("[project]"));
    parser.addOption({QStringLiteral("demo"),QStringLiteral("Open the built-in assembly example")});
    parser.addOption({QStringLiteral("dress-form-demo"),QStringLiteral("Open a two-panel garment around the dress form")});
    parser.addOption({QStringLiteral("render"),QStringLiteral("Render the application window to PNG and exit"),QStringLiteral("path")});
    parser.addOption({QStringLiteral("steps"),QStringLiteral("Solver iterations before rendering (0–2400)"),QStringLiteral("count"),QStringLiteral("0")});
    parser.addOption({QStringLiteral("validate"),QStringLiteral("Validate and mesh a project, then exit")});
    parser.process(*app);
    bool valid=false; const int steps=parser.value(QStringLiteral("steps")).toInt(&valid);
    if (!valid || steps<0 || steps>2400 || parser.positionalArguments().size()>1 ||
        ((parser.isSet(QStringLiteral("demo")) || parser.isSet(QStringLiteral("dress-form-demo"))) && !parser.positionalArguments().isEmpty()) ||
        (parser.isSet(QStringLiteral("demo")) && parser.isSet(QStringLiteral("dress-form-demo")))) parser.showHelp(2);
    if (parser.isSet(QStringLiteral("validate")))
    {
        Seamly3D::Project project; QString error;
        if (parser.isSet(QStringLiteral("dress-form-demo"))) project=Seamly3D::dressFormDemoProject();
        else if (parser.isSet(QStringLiteral("demo"))) project=Seamly3D::demoProject();
        else if (parser.positionalArguments().isEmpty() || !Seamly3D::load(parser.positionalArguments().first(),project,error))
        { QTextStream(stderr) << (error.isEmpty() ? QStringLiteral("A project path or --demo is required") : error) << '\n'; return 1; }
        Seamly3D::Assembly assembly;
        if (!assembly.reset(project,error)) { QTextStream(stderr) << error << '\n'; return 1; }
        for (int i=0; i<steps; i+=20) assembly.step(qMin(20,steps-i));
        QTextStream(stdout) << "Pieces: " << project.pieces.size() << "; seams: " << project.seams.size() << "; mean seam gap (mm): " << assembly.seamError() << '\n';
        return 0;
    }
    Seamly3D::Window window;
    if (!parser.positionalArguments().isEmpty() && !window.openProject(parser.positionalArguments().first())) return 1;
    if (parser.isSet(QStringLiteral("demo"))) window.showDemo();
    if (parser.isSet(QStringLiteral("dress-form-demo"))) window.showDressFormDemo();
    window.show();
    QTimer renderTimer;
    QElapsedTimer renderDeadline;
    if (parser.isSet(QStringLiteral("render")))
    {
        for (int i=0; i<steps; i+=20) window.advance(qMin(20,steps-i));
        renderDeadline.start(); renderTimer.setInterval(50);
        QObject::connect(&renderTimer,&QTimer::timeout,&window,[&]() {
            if (!window.rendererError().isEmpty())
            {
                renderTimer.stop();
                QTextStream(stderr) << window.rendererError() << '\n'; app->exit(1);
            }
            else if (window.rendererReady())
            {
                renderTimer.stop();
                QTextStream(stdout) << "3D backend: " << window.graphicsBackend() << '\n';
                app->exit(window.render(parser.value(QStringLiteral("render"))) ? 0 : 1);
            }
            else if (renderDeadline.elapsed()>10000)
            {
                renderTimer.stop();
                QTextStream(stderr) << "The 3D renderer did not produce a frame within 10 seconds.\n"; app->exit(1);
            }
        });
        renderTimer.start();
    }
    return app->exec();
}
