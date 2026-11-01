// SPDX-License-Identifier: GPL-3.0-or-later
#include "../../app/seamly3d/model.h"
#include <QtTest>
#include "../../app/seamly3d/window.h"
#include "../../app/seamly3d/scenegeometry.h"
#include "../../app/seamly3d/dressform.h"
#include <QGroupBox>
#include <QTabWidget>
#include <QStyleHints>
#include <QDir>
#include "../../app/seamly3d/viewport.h"
#include <QQuickWidget>
#include <QQuickItem>
#include <QJSValue>
#include <QAction>
#include <QUndoStack>
#include <QCheckBox>
#include <QLineEdit>
#include <QLabel>
#include <cstring>
#include <QComboBox>
#include <QPushButton>
#include <QListWidget>
#include <QDoubleSpinBox>
#include <QJsonArray>
#include <QTemporaryDir>
#include <cmath>
#include <algorithm>
using namespace Seamly3D;
static QJsonObject legacyProject(const Project &project,int version)
{
    auto json=encode(project); json["version"]=version; auto rows=json["seams"].toArray();
    for (int i=0;i<rows.size();++i) { auto row=rows[i].toObject(); for (const auto key : {"startA","endA","startB","endB"}) row[key]=row[key].toDouble()*BoundarySamples; rows[i]=row; }
    json["seams"]=rows; return json;
}

class Seamly3DTest : public QObject
{
    Q_OBJECT
private slots:
    void projectRoundTrip()
    {
        const auto input=demoProject(); Project output; QString error;
        QVERIFY2(decode(encode(input),output,error),qPrintable(error));
        QCOMPARE(encode(output),encode(input));
        QTemporaryDir directory; QVERIFY(directory.isValid());
        const auto path=directory.filePath(QStringLiteral("assembly.sm3d"));
        QVERIFY2(save(path,input,error),qPrintable(error));
        QVERIFY2(load(path,output,error),qPrintable(error)); QCOMPARE(encode(output),encode(input));
        auto invalid=input; invalid.pieces[0].outline.clear();
        QVERIFY(!save(path,invalid,error));
        QVERIFY(load(path,output,error)); QCOMPARE(encode(output),encode(input));
    }
    void invalidInputIsTransactional_data()
    {
        QTest::addColumn<QString>("kind");
        for (const auto &kind : {"version","units","duplicate","coordinate","bowtie","missing-piece","endpoint","direction","self-seam","pose","missing-array"})
            QTest::newRow(kind) << QString::fromLatin1(kind);
    }
    void invalidInputIsTransactional()
    {
        QFETCH(QString,kind);
        const auto baseline=demoProject(); auto json=encode(baseline);
        auto pieces=json["pieces"].toArray(); auto piece=pieces[0].toObject();
        auto seams=json["seams"].toArray(); auto seam=seams[0].toObject();
        if (kind=="version") json["version"]=99;
        if (kind=="units") json["units"]="cm";
        if (kind=="duplicate") { auto second=pieces[1].toObject(); second["id"]=piece["id"]; pieces[1]=second; }
        if (kind=="coordinate") piece["outline"]=QJsonArray{QJsonArray{"invalid",0},QJsonArray{1,0},QJsonArray{1,1}};
        if (kind=="bowtie") piece["outline"]=QJsonArray{QJsonArray{0,0},QJsonArray{10,10},QJsonArray{0,10},QJsonArray{10,0}};
        if (kind=="missing-piece") seam["a"]="absent";
        if (kind=="endpoint") seam["startA"]=1.5;
        if (kind=="direction") seam["reverse"]="yes";
        if (kind=="self-seam") seam["b"]=seam["a"];
        if (kind=="pose") piece["translation"]=QJsonArray{0,0};
        pieces[0]=piece; seams[0]=seam; json["pieces"]=pieces; json["seams"]=seams;
        if (kind=="missing-array") json.remove("seams");
        auto destination=baseline; QString error;
        QVERIFY(!decode(json,destination,error)); QVERIFY(!error.isEmpty());
        QCOMPARE(encode(destination),encode(baseline));
    }
    void boundaryAndArcLengths()
    {
        Piece p; p.outline={{0,0},{100,0},{100,100},{0,100}};
        const auto boundary=resample(p.outline);
        QCOMPARE(boundary.size(),BoundarySamples); QCOMPARE(boundary[0],QPointF(0,0)); QCOMPARE(boundary[16],QPointF(100,0));
        QCOMPARE(seamLength(p,48,16),200.0); QCOMPARE(seamLength(p,5,5),400.0);
        QCOMPARE(arcSteps(63,1),2);
    }
    void meshConservesAreaAndWinding_data()
    {
        QTest::addColumn<bool>("reverse"); QTest::newRow("ccw") << false; QTest::newRow("cw") << true;
    }
    void meshConservesAreaAndWinding()
    {
        QFETCH(bool,reverse); Project project; Piece p; p.id="square"; p.name="Square";
        p.outline={{0,0},{100,0},{100,100},{0,100}};
        if (reverse) std::reverse(p.outline.begin(),p.outline.end());
        p.translation={10,20,30}; project.pieces << p;
        Assembly assembly; QString error; QVERIFY2(assembly.reset(project,error),qPrintable(error));
        const auto &mesh=assembly.meshes()[0]; double area=0;
        QVERIFY(mesh.triangles.size()>2);
        for (const auto &e : mesh.edges) QVERIFY(e.length<=project.meshSize*1.0001);
        for (const auto &t : mesh.triangles)
        {
            const auto normal=QVector3D::crossProduct(mesh.rest[t[1]]-mesh.rest[t[0]],mesh.rest[t[2]]-mesh.rest[t[0]]);
            QVERIFY(normal.z()>0); area+=normal.length()/2;
        }
        QVERIFY(std::abs(area-10000)<0.1);
        QCOMPARE(mesh.vertices[0],mesh.rest[0]+p.translation);
        const auto before=mesh.vertices; assembly.step(10); QCOMPARE(assembly.meshes()[0].vertices,before);
    }
    void stitchesConverge_data()
    {
        QTest::addColumn<bool>("reverse"); QTest::addColumn<bool>("fullLoop");
        QTest::newRow("parallel-arcs") << false << false;
        QTest::newRow("reversed-arcs") << true << false;
        QTest::newRow("parallel-loop") << false << true;
        QTest::newRow("reversed-loop") << true << true;
    }
    void stitchesConverge()
    {
        QFETCH(bool,reverse); QFETCH(bool,fullLoop);
        auto project=demoProject(); auto &s=project.seams[0]; s.reverse=reverse;
        if (fullLoop) s.startA=s.endA=s.startB=s.endB=0;
        Assembly assembly; QString error; QVERIFY2(assembly.reset(project,error),qPrintable(error));
        const auto original=assembly.seamError(); QVERIFY(original>50);
        for (int i=0; i<100; ++i) assembly.step(10);
        QVERIFY2(assembly.seamError()<original*0.03,qPrintable(QString::number(assembly.seamError())));
        for (const auto &mesh : assembly.meshes()) for (const auto &v : mesh.vertices)
            QVERIFY(std::isfinite(v.x()) && std::isfinite(v.y()) && std::isfinite(v.z()));
        const auto previous=assembly.meshes()[0].vertices;
        auto invalid=project; invalid.pieces[0].outline.clear();
        QVERIFY(!assembly.reset(invalid,error)); QCOMPARE(assembly.meshes()[0].vertices,previous);
    }
    void geometryRefresh()
    {
        const auto original=demoProject(); auto incoming=original;
        incoming.pieces[0].translation={50,0,0};
        auto result=mergeGeometry(original,incoming);
        QCOMPARE(result.seams.size(),1); QCOMPARE(result.pieces[0].translation,original.pieces[0].translation);
        incoming.pieces[0].outline[0]+=QPointF(1,0);
        result=mergeGeometry(original,incoming); QVERIFY(result.seams.isEmpty());
        incoming=original; incoming.pieces.removeLast();
        result=mergeGeometry(original,incoming); QVERIFY(result.seams.isEmpty());
        incoming=original; incoming.source=QStringLiteral("Another pattern");
        result=mergeGeometry(original,incoming); QVERIFY(result.seams.isEmpty());
    }
    void editorWorkflow()
    {
        if (!qEnvironmentVariableIsSet("SEAMLY3D_GPU_TESTS")) QSKIP("Set SEAMLY3D_GPU_TESTS=1 on a real graphics platform for renderer/editor acceptance.");
        QTemporaryDir directory; QVERIFY(directory.isValid()); QString error;
        auto project=demoProject(); project.seams.clear();
        const auto path=directory.filePath(QStringLiteral("editor.sm3d")); QVERIFY(save(path,project,error));
        Window window; QVERIFY(window.openProject(path)); window.show();
        auto run=window.findChild<QPushButton *>(QStringLiteral("assembleButton"));
        auto add=window.findChild<QPushButton *>(QStringLiteral("addSeamButton"));
        auto seams=window.findChild<QListWidget *>(QStringLiteral("seamList"));
        QVERIFY(run && add && seams); QVERIFY(!run->isEnabled());
        auto tabs=window.findChild<QTabWidget *>(QStringLiteral("inspectorTabs")); QVERIFY(tabs); QCOMPARE(tabs->count(),4);
        tabs->setCurrentIndex(2);
        QTest::mouseClick(add,Qt::LeftButton); QCOMPARE(seams->count(),1); QVERIFY(run->isEnabled()); QVERIFY(window.isWindowModified());
        QTest::mouseClick(run,Qt::LeftButton); QCOMPARE(run->text(),QStringLiteral("Pause"));
        QTest::mouseClick(run,Qt::LeftButton); QCOMPARE(run->text(),QStringLiteral("Assemble"));
        auto view=window.findChild<Viewport *>(QStringLiteral("assemblyViewport")); QVERIFY(view);
        QTRY_VERIFY_WITH_TIMEOUT(window.rendererReady(),10000);
        QVERIFY2(window.rendererError().isEmpty(),qPrintable(window.rendererError()));
#ifdef Q_OS_MAC
        QCOMPARE(window.graphicsBackend(),QStringLiteral("Metal"));
#endif
        window.advance(100); QVERIFY(window.render(directory.filePath(QStringLiteral("preview.png"))));
        auto x=window.findChild<QDoubleSpinBox *>(QStringLiteral("pose0")); QVERIFY(x); x->setValue(25); QCOMPARE(x->value(),25.0);
        seams->setCurrentRow(0);
        auto remove=window.findChild<QPushButton *>(QStringLiteral("removeSeamButton")); QVERIFY(remove);
        QTest::mouseClick(remove,Qt::LeftButton); QCOMPARE(seams->count(),0); QVERIFY(!run->isEnabled());
        window.setWindowModified(false); window.close();
    }
    void renderGeometry()
    {
        const auto project=demoProject(); Assembly assembly; QString error;
        QVERIFY2(assembly.reset(project,error),qPrintable(error));
        ScenePiece piece(project.pieces[0].id);
        piece.synchronize(assembly.meshes()[0],project.pieces[0],project.seams,0);
        auto surface=piece.surface(); const auto &mesh=assembly.meshes()[0];
        QCOMPARE(surface->stride(),6*int(sizeof(float)));
        QCOMPARE(surface->vertexData().size(),mesh.vertices.size()*surface->stride());
        QCOMPARE(surface->indexData().size(),mesh.triangles.size()*3*int(sizeof(quint32)));
        QCOMPARE(surface->primitiveType(),QQuick3DGeometry::PrimitiveType::Triangles);
        const auto data=surface->vertexData();
        for (int i=0; i<mesh.vertices.size(); ++i)
        {
            float values[6]; std::memcpy(values,data.constData()+i*surface->stride(),sizeof(values));
            QCOMPARE(QVector3D(values[0],values[1],values[2]),mesh.vertices[i]);
            QVERIFY(std::abs(QVector3D(values[3],values[4],values[5]).length()-1)<1e-5);
            for (int k=0; k<3; ++k) { QVERIFY(values[k]>=surface->boundsMin()[k]); QVERIFY(values[k]<=surface->boundsMax()[k]); }
        }
        QCOMPARE(piece.wire()->indexData().size(),mesh.edges.size()*2*int(sizeof(quint32)));
        QVERIFY(piece.hasSewing()); QVERIFY(!piece.sewing()->indexData().isEmpty());
        const auto initial=surface->vertexData(); assembly.step(50);
        piece.synchronize(assembly.meshes()[0],project.pieces[0],{},0);
        QCOMPARE(piece.surface(),surface); QVERIFY(surface->vertexData()!=initial); QVERIFY(!piece.hasSewing());
        QVERIFY(piece.sewing()->indexData().isEmpty());
    }
    void nativeViewport()
    {
        if (!qEnvironmentVariableIsSet("SEAMLY3D_GPU_TESTS")) QSKIP("Native GPU test is opt-in.");
        Project project; Piece front; front.id="front"; front.name="Front";
        front.outline={{-50,-50},{50,-50},{50,50},{-50,50}}; front.translation={0,0,20};
        Piece back=front; back.id="back"; back.name="Back"; back.translation={0,0,-20};
        project.pieces={front,back}; Assembly assembly; QString error; QVERIFY(assembly.reset(project,error));
        Viewport view(&project,&assembly); view.resize(600,500); view.synchronize(); view.fit(); view.show();
        QTRY_VERIFY_WITH_TIMEOUT(view.rendererReady(),10000);
        QVERIFY2(view.rendererError().isEmpty(),qPrintable(view.rendererError()));
        auto quick=view.findChild<QQuickWidget *>(); QVERIFY(quick); QCOMPARE(quick->status(),QQuickWidget::Ready);
        QVERIFY(view.pieces().size()==2); auto piece=view.pieces()[0].value<ScenePiece *>(); QVERIFY(piece);
        // Looking straight down -Z, the front panel must occlude the later back panel.
        view.orbit(50,40);
        QTest::qWait(100);
        QTest::mouseClick(quick,Qt::LeftButton,Qt::NoModifier,QPoint(300,250));
        QTRY_COMPARE(view.selectedId(),QStringLiteral("front"));
        QCOMPARE(view.selectedName(),QStringLiteral("Front"));
        const auto frame=view.capture(); QVERIFY(!frame.isNull());
        const auto sample=frame.pixelColor(frame.width()/2,frame.height()/2);
        QVERIFY2(sample.green()>sample.red() && sample.blue()>sample.red(),qPrintable(sample.name()));
        const auto before=assembly.meshes()[0].vertices;
        const float magnification=view.magnification(); view.zoom(120); QVERIFY(view.magnification()>magnification);
        const float yaw=view.yaw(); view.orbit(20,10); QVERIFY(view.yaw()!=yaw);
        view.pan(20,10); QVERIFY(!view.cameraPan().isNull());
        view.setWireframe(true); QVERIFY(view.wireframe()); view.fit(); QVERIFY(view.cameraPan().isNull());
        view.synchronize(); QCOMPARE(view.pieces()[0].value<ScenePiece *>(),piece); QCOMPARE(view.selectedId(),QStringLiteral("front"));
        QCOMPARE(assembly.meshes()[0].vertices,before);
        view.selectPiece(QStringLiteral("missing")); QCOMPARE(view.selectedId(),QStringLiteral("front"));
        view.selectPiece(QString()); QVERIFY(view.selectedId().isEmpty());
        project.pieces.clear(); assembly=Assembly(); view.synchronize(); QVERIFY(view.pieces().isEmpty());
    }
    void formPersistence()
    {
        Project form; form.dressForm.enabled=true; form.dressForm.bust=1040; form.dressForm.stand=false;
        Project loaded; QString error;
        QVERIFY2(decode(encode(form),loaded,error),qPrintable(error)); QCOMPARE(loaded.dressForm,form.dressForm);
        QTemporaryDir directory; QVERIFY(directory.isValid());
        const auto path=directory.filePath(QStringLiteral("form.sm3d")); QVERIFY(save(path,form,error));
        QVERIFY(load(path,loaded,error)); QCOMPARE(loaded.dressForm,form.dressForm);
        QVERIFY(loaded.pieces.isEmpty()); Assembly assembly; QVERIFY(assembly.reset(loaded,error));
        auto invalid=encode(form); auto settings=invalid["dressForm"].toObject(); settings["height"]=-20; invalid["dressForm"]=settings;
        QVERIFY(!decode(invalid,loaded,error)); QCOMPARE(loaded.dressForm,form.dressForm);
        invalid=encode(form); settings=invalid["dressForm"].toObject(); settings["bust"]="invalid"; invalid["dressForm"]=settings;
        QVERIFY(!decode(invalid,loaded,error));
        auto legacy=legacyProject(demoProject(),1); legacy.remove("dressForm");
        QVERIFY(decode(legacy,loaded,error)); QVERIFY(!loaded.dressForm.enabled);
        auto incoming=demoProject(); auto current=incoming; current.dressForm=form.dressForm;
        QCOMPARE(mergeGeometry(current,incoming).dressForm,form.dressForm);
    }
    void formGeometry()
    {
        DressForm form; const auto mesh=dressFormMesh(form);
        QVERIFY(mesh.vertices.size()>1000); QVERIFY(mesh.triangles.size()>2000);
        QMap<QPair<int,int>,int> edges;
        for (const auto &t : mesh.triangles)
        {
            for (int index : t) QVERIFY(index>=0 && index<mesh.vertices.size());
            QVERIFY(QVector3D::crossProduct(mesh.vertices[t[1]]-mesh.vertices[t[0]],mesh.vertices[t[2]]-mesh.vertices[t[0]]).length()>1e-4f);
            for (int i=0; i<3; ++i) { int a=t[i],b=t[(i+1)%3]; if (a>b) qSwap(a,b); ++edges[qMakePair(a,b)]; }
        }
        for (int count : edges) QCOMPARE(count,2); // Closed manifold, including both caps.
        const double heights[]={0.67,0.42,0.22}; const double circumferences[]={form.bust,form.waist,form.hips};
        for (int i=0; i<3; ++i)
        {
            const auto radius=dressFormRadii(form,dressFormBottom(form)+form.height*heights[i]);
            double length=0; QPointF previous(radius.x(),0);
            for (int j=1; j<=720; ++j)
            {
                const double angle=6.283185307179586*j/720; const QPointF p(radius.x()*std::cos(angle),radius.y()*std::sin(angle));
                length+=QLineF(previous,p).length(); previous=p;
            }
            QVERIFY(std::abs(length-circumferences[i])<0.1);
        }
        auto larger=form; larger.bust=1200;
        QVERIFY(dressFormRadii(larger,dressFormBottom(form)+form.height*0.67).x()>dressFormRadii(form,dressFormBottom(form)+form.height*0.67).x());
    }
    void formContactAndPlacement()
    {
        auto project=dressFormDemoProject(); const auto profile=dressFormProfile(project.dressForm);
        const auto center=QVector3D(0,0,0); const auto projected=outsideDressForm(profile,center,3);
        QVERIFY(projected.z()>90); QVERIFY((outsideDressForm(profile,projected,3)-projected).length()<0.01f);
        const auto distant=QVector3D(1000,0,1000); QCOMPARE(outsideDressForm(profile,distant,3),distant);
        const auto beyond=QVector3D(0,float(dressFormTop(project.dressForm)+100),0); QCOMPARE(outsideDressForm(profile,beyond,3),beyond);
        for (double angle : {0.,90.,180.,270.})
        {
            auto piece=project.pieces[0]; placeAroundDressForm(piece,project.dressForm,angle);
            Project single; single.dressForm=project.dressForm; single.pieces={piece};
            Assembly placed; QString error; QVERIFY(placed.reset(single,error));
            for (const auto &v : placed.meshes()[0].vertices) QVERIFY((outsideDressForm(profile,v,3)-v).length()<0.01f);
        }
        // Start inside intentionally and ensure each accepted step ends outside.
        project.pieces[0].translation={0,0,0}; project.pieces[1].translation={0,0,0};
        Assembly assembly; QString error; QVERIFY(assembly.reset(project,error));
        for (int i=0; i<10; ++i)
        {
            assembly.step(5);
            for (const auto &mesh : assembly.meshes()) for (const auto &v : mesh.vertices)
                QVERIFY2((outsideDressForm(profile,v,3)-v).length()<0.02f,"A vertex remains inside the dress form");
        }
        auto plain=demoProject(); auto noContact=plain; noContact.dressForm.enabled=true; noContact.dressForm.contact=false;
        Assembly a,b; QVERIFY(a.reset(plain,error)); QVERIFY(b.reset(noContact,error)); a.step(20); b.step(20);
        QCOMPARE(a.meshes()[0].vertices,b.meshes()[0].vertices);
    }
    void dressFormScene()
    {
        if (!qEnvironmentVariableIsSet("SEAMLY3D_GPU_TESTS")) QSKIP("Native GPU test is opt-in.");
        Window window; window.show();
        QTRY_VERIFY_WITH_TIMEOUT(window.rendererReady(),10000);
        auto view=window.findChild<Viewport *>(); QVERIFY(view); QVERIFY(view->dressFormEnabled());
        QVERIFY(!view->dressFormGeometry()->vertexData().isEmpty());
        auto bust=window.findChild<QDoubleSpinBox *>(QStringLiteral("formMeasure0")); QVERIFY(bust);
        QCOMPARE(bust->value(),92.0); const auto before=view->dressFormGeometry()->boundsMax(); bust->setValue(110);
        QVERIFY(view->dressFormGeometry()->boundsMax().z()>before.z());
        bust->setValue(92);
        const auto image=view->capture(); QVERIFY(!image.isNull());
        int beige=0;
        for (int y=0; y<image.height(); y+=4) for (int x=0; x<image.width(); x+=4)
        { const auto p=image.pixelColor(x,y); if (p.red()>100 && p.green()>90 && p.red()>p.blue()*1.1) ++beige; }
        QVERIFY(beige>300);
        const auto output=qEnvironmentVariable("SEAMLY3D_FORM_CAPTURE"); if (!output.isEmpty()) QVERIFY(window.render(output));
        window.setWindowModified(false); window.close();
    }
    void dressFormEditorPlacement()
    {
        if (!qEnvironmentVariableIsSet("SEAMLY3D_GPU_TESTS")) QSKIP("Native GPU test is opt-in.");
        QTemporaryDir directory; QString error; const auto path=directory.filePath(QStringLiteral("dress.sm3d"));
        QVERIFY(save(path,dressFormDemoProject(),error)); Window window; QVERIFY(window.openProject(path)); window.show();
        QTRY_VERIFY_WITH_TIMEOUT(window.rendererReady(),10000);
        auto x=window.findChild<QDoubleSpinBox *>(QStringLiteral("pose0")); QVERIFY(x); x->setValue(600);
        auto arrange=window.findChild<QPushButton *>(QStringLiteral("arrangeOnFormButton")); QVERIFY(arrange);
        arrange->click(); QVERIFY(std::abs(x->value())<0.1); QVERIFY(window.isWindowModified());
        const auto capture=qEnvironmentVariable("SEAMLY3D_FORM_CAPTURE");
        if (!capture.isEmpty()) { window.advance(300); QVERIFY(window.render(capture+QStringLiteral("-mounted.png"))); }
        window.setWindowModified(false); window.close();
    }
    void placementHandles()
    {
        if (!qEnvironmentVariableIsSet("SEAMLY3D_GPU_TESTS")) QSKIP("Native GPU test is opt-in.");
        Window window; window.showDressFormDemo(); window.show();
        QTRY_VERIFY_WITH_TIMEOUT(window.rendererReady(),10000);
        auto view=window.findChild<Viewport *>(); QVERIFY(view);
        auto quick=view->findChild<QQuickWidget *>(); QVERIFY(quick);
        view->selectPiece(QStringLiteral("front")); view->setView(1); view->setToolMode(1);
        auto handles=quick->rootObject()->findChild<QQuickItem *>(QStringLiteral("placementHandles")); QVERIFY(handles);
        auto geometry=[&]() { return handles->property("geometry").value<QJSValue>(); };
        QTRY_VERIFY(geometry().property(QStringLiteral("length")).toInt()>=2);
        const auto axis=geometry().property(0); QCOMPARE(axis.property(QStringLiteral("axis")).toInt(),0);
        const auto center=axis.property(QStringLiteral("center")),end=axis.property(QStringLiteral("end"));
        const QPoint start(qRound(center.property("x").toNumber()*0.3+end.property("x").toNumber()*0.7),
                           qRound(center.property("y").toNumber()*0.3+end.property("y").toNumber()*0.7));
        auto x=window.findChild<QDoubleSpinBox *>(QStringLiteral("pose0")); QVERIFY(x);
        const double before=x->value(),distance=40/view->magnification();
        QTest::mousePress(quick,Qt::LeftButton,Qt::NoModifier,start);
        QTest::mouseMove(quick,start+QPoint(40,0),30);
        QTest::mouseRelease(quick,Qt::LeftButton,Qt::NoModifier,start+QPoint(40,0));
        QVERIFY2(std::abs(x->value()-before-distance)<0.2,qPrintable(QString::number(x->value()-before)));
        QCOMPARE(view->yaw(),0.f); QVERIFY(window.isWindowModified());
        // Escape is transactional, including the pre-existing modified state.
        window.setWindowModified(false); const double placed=x->value();
        QVERIFY(view->beginTransform()); view->transformBy(0,80); QVERIFY(x->value()>placed+79);
        view->endTransform(true); QCOMPARE(x->value(),placed); QVERIFY(!window.isWindowModified());
        view->setToolMode(2); auto rotation=window.findChild<QDoubleSpinBox *>(QStringLiteral("pose5")); QVERIFY(rotation);
        QTRY_COMPARE(geometry().property(QStringLiteral("length")).toInt(),1);
        const auto ring=geometry().property(0); QCOMPARE(ring.property("axis").toInt(),2);
        const auto ringCenter=ring.property("center");
        auto ringPoint=[&](double angle) {
            return QPoint(qRound(ringCenter.property("x").toNumber()+ring.property("ux").toNumber()*std::cos(angle)+ring.property("vx").toNumber()*std::sin(angle)),
                          qRound(ringCenter.property("y").toNumber()+ring.property("uy").toNumber()*std::cos(angle)+ring.property("vy").toNumber()*std::sin(angle)));
        };
        const double angle=rotation->value();
        QTest::mousePress(quick,Qt::LeftButton,Qt::NoModifier,ringPoint(0.3));
        QTest::mouseMove(quick,ringPoint(0.8),30);
        QTest::mouseRelease(quick,Qt::LeftButton,Qt::NoModifier,ringPoint(0.8));
        QVERIFY(std::abs(rotation->value()-angle-0.5*180/M_PI)<1.5);
        const double rotated=rotation->value();
        QTest::mousePress(quick,Qt::LeftButton,Qt::NoModifier,ringPoint(0.3));
        QTest::mouseMove(quick,ringPoint(1.0),30); QVERIFY(rotation->value()!=rotated);
        QTest::keyClick(quick,Qt::Key_Escape);
        QTest::mouseRelease(quick,Qt::LeftButton,Qt::NoModifier,ringPoint(1.0));
        QCOMPARE(rotation->value(),rotated);
        const auto origin=view->placementOrigin(); view->setView(2); QCOMPARE(view->yaw(),180.f);
        view->setView(3); QCOMPARE(view->yaw(),90.f); view->setView(4); QCOMPARE(view->yaw(),-90.f);
        view->setView(5); QCOMPARE(view->pitch(),-89.f); view->setView(0); QCOMPARE(view->yaw(),-25.f);
        QCOMPARE(view->placementOrigin(),origin); QCOMPARE(x->value(),placed);
        // View shortcuts belong to the canvas and must not intercept numeric entry.
        x->setFocus(); x->selectAll(); QTest::keyClicks(x,"123"); QTest::keyClick(x,Qt::Key_Return); QCOMPARE(x->value(),123.0);
        window.setWindowModified(false); window.close();
    }
    void historyWorkflow()
    {
        if (!qEnvironmentVariableIsSet("SEAMLY3D_GPU_TESTS")) QSKIP("Native GPU test is opt-in.");
        QTemporaryDir temporary; QString error; const auto path=temporary.filePath("history.sm3d");
        QVERIFY(save(path,dressFormDemoProject(),error)); Window window; QVERIFY(window.openProject(path)); window.show();
        QTRY_VERIFY_WITH_TIMEOUT(window.rendererReady(),10000);
        auto history=window.findChild<QUndoStack *>("assemblyHistory"); QVERIFY(history); QCOMPARE(history->count(),0); QVERIFY(!window.isWindowModified());
        auto view=window.findChild<Viewport *>(); view->selectPiece("front");
        auto x=window.findChild<QDoubleSpinBox *>("pose0"); const auto initial=x->value(); x->setValue(initial+10);
        QCOMPARE(history->count(),1); QVERIFY(window.isWindowModified()); history->undo(); QCOMPARE(x->value(),initial); QVERIFY(!window.isWindowModified());
        history->redo(); QCOMPARE(x->value(),initial+10);
        auto bust=window.findChild<QDoubleSpinBox *>("formMeasure0"); const double initialBust=bust->value(); bust->setValue(initialBust+1);
        QCOMPARE(history->count(),2); history->undo(); QCOMPARE(bust->value(),initialBust); history->redo();
        window.findChild<QPushButton *>("arrangeOnFormButton")->click(); QCOMPARE(history->count(),3);
        auto seams=window.findChild<QListWidget *>("seamList"); const int originalSeams=seams->count();
        window.findChild<QPushButton *>("addSeamButton")->click(); QCOMPARE(seams->count(),originalSeams+1); QCOMPARE(history->count(),4);
        seams->setCurrentRow(seams->count()-1); window.findChild<QPushButton *>("removeSeamButton")->click(); QCOMPARE(history->count(),5);
        history->undo(); QCOMPARE(seams->count(),originalSeams+1); history->redo(); QCOMPARE(seams->count(),originalSeams);
        view->setToolMode(1); const int beforeDrag=history->count(); const double beforeX=x->value();
        QVERIFY(view->beginTransform()); view->transformBy(0,5); view->transformBy(0,15); view->transformBy(0,25);
        QCOMPARE(history->count(),beforeDrag); view->endTransform(false); QCOMPARE(history->count(),beforeDrag+1);
        history->undo(); QCOMPARE(x->value(),beforeX); history->redo(); QCOMPARE(x->value(),beforeX+25);
        const int noOpCount=history->count(); QVERIFY(view->beginTransform()); view->endTransform(false); QCOMPARE(history->count(),noOpCount);
        QVERIFY(window.saveToPath(path)); QVERIFY(!window.isWindowModified()); QVERIFY(history->isClean());
        QVERIFY(view->beginTransform()); view->transformBy(0,35); view->endTransform(true); QVERIFY(!window.isWindowModified()); QCOMPARE(history->count(),noOpCount);
        history->undo(); QVERIFY(window.isWindowModified()); history->redo(); QVERIFY(!window.isWindowModified());
        Project saved; QVERIFY(load(path,saved,error)); const auto savedJson=encode(saved);
        auto incoming=saved; incoming.pieces[0].outline[0]+=QPointF(5,0);
        const auto updatedPath=temporary.filePath("updated.sm3d"); QVERIFY(save(updatedPath,incoming,error));
        QVERIFY(window.importFromPath(updatedPath)); QVERIFY(window.isWindowModified()); history->undo();
        const auto checkPath=temporary.filePath("restored.sm3d"); QVERIFY(window.saveToPath(checkPath)); Project restored; QVERIFY(load(checkPath,restored,error)); QCOMPARE(encode(restored),savedJson);
        history->undo(); bust->setValue(bust->value()+1); QVERIFY(!history->canRedo());
        QVERIFY(window.saveToPath(path)); QVERIFY(window.openProject(path)); QCOMPARE(history->count(),0); QVERIFY(history->isClean());
        auto undo=window.findChild<QAction *>("undoAction"); QVERIFY(undo); QVERIFY(!undo->isEnabled());
        window.setWindowModified(false); window.close();
    }
    void inspectionWorkflow()
    {
        if (!qEnvironmentVariableIsSet("SEAMLY3D_GPU_TESTS")) QSKIP("Native GPU test is opt-in.");
        QTemporaryDir temporary; QString error; auto project=dressFormDemoProject();
        for (int i=0;i<2;++i) { auto piece=project.pieces[i]; piece.id=QString("extra%1").arg(i); piece.name=QString("Lining %1").arg(i); piece.translation.setX(150*(i+1)); project.pieces<<piece; }
        const auto path=temporary.filePath("inspection.sm3d"); QVERIFY(save(path,project,error));
        Window window; QVERIFY(window.openProject(path)); window.show(); QTRY_VERIFY_WITH_TIMEOUT(window.rendererReady(),10000);
        auto view=window.findChild<Viewport *>(); auto list=window.findChild<QListWidget *>("scenePieceList"); QVERIFY(list); QCOMPARE(list->count(),4);
        auto history=window.findChild<QUndoStack *>("assemblyHistory");
        auto selector=window.findChild<QComboBox *>("placementPiece");
        for (int i=0;i<list->count();++i) QCOMPARE(list->item(i)->checkState(),Qt::Checked);
        list->setCurrentRow(2); QCOMPARE(view->selectedId(),QString("extra0")); QCOMPARE(selector->currentData().toString(),view->selectedId());
        list->setFocus(); QTest::keyClick(list,Qt::Key_Space); QVERIFY(!view->pieceVisible("extra0"));
        QCOMPARE(window.findChild<QPushButton *>("assembleButton")->text(),QString("Assemble"));
        QTest::keyClick(list,Qt::Key_Space); QVERIFY(view->pieceVisible("extra0"));
        view->selectPiece("back"); QCOMPARE(list->currentRow(),1);
        window.findChild<QPushButton *>("isolatePieceButton")->click();
        for (int i=0;i<4;++i) QCOMPARE(view->pieceVisible(project.pieces[i].id),i==1);
        list->item(1)->setCheckState(Qt::Unchecked); QVERIFY(!view->selectedVisible()); view->setToolMode(1); QVERIFY(!view->beginTransform());
        window.findChild<QPushButton *>("showAllPiecesButton")->click(); for (const auto &piece : project.pieces) QVERIFY(view->pieceVisible(piece.id));
        auto transparent=window.findChild<QCheckBox *>("transparentPiece"); transparent->setChecked(true); QVERIFY(view->pieceTransparent("back"));
        for (auto value : view->pieces()) { auto scene=value.value<ScenePiece *>(); QCOMPARE(scene->ghosted(),scene->pieceId()=="back"); }
        window.advance(40); QVERIFY(view->pieceTransparent("back")); QCOMPARE(history->count(),0); QVERIFY(!window.isWindowModified());
        const auto output=temporary.filePath("unchanged.sm3d"); QVERIFY(window.saveToPath(output)); Project unchanged; QVERIFY(load(output,unchanged,error)); QCOMPARE(encode(unchanged),encode(project));
        view->selectPiece("extra1"); list->item(2)->setCheckState(Qt::Unchecked);
        auto x=window.findChild<QDoubleSpinBox *>("pose0"); x->setValue(x->value()+5); history->undo(); QVERIFY(!view->pieceVisible("extra0")); QVERIFY(view->pieceTransparent("back"));
        QVERIFY(window.openProject(output)); QVERIFY(!view->pieceTransparent("back")); for (const auto &piece : project.pieces) QVERIFY(view->pieceVisible(piece.id));
        view->selectPiece(QString()); QCOMPARE(list->currentRow(),-1); QVERIFY(!transparent->isEnabled());
        window.setWindowModified(false); window.close();
    }
    void seamMetadataAndPause()
    {
        auto project=demoProject(); auto &seam=project.seams[0]; seam.name="Named side"; seam.enabled=false;
        Project decoded; QString error; QVERIFY(decode(encode(project),decoded,error)); QCOMPARE(encode(decoded),encode(project));
        QCOMPARE(encode(project).value("version").toInt(),4);
        Assembly inactive; QVERIFY(inactive.reset(project,error)); const auto before=inactive.meshes()[0].vertices;
        inactive.step(20); QCOMPARE(inactive.seamError(),0.0);
        for (int i=0;i<before.size();++i) QVERIFY((inactive.meshes()[0].vertices[i]-before[i]).length()<0.001f);
        seam.enabled=true; Assembly active; QVERIFY(active.reset(project,error)); QVERIFY(active.seamError()>1);
        auto legacy=legacyProject(project,2); auto seams=legacy["seams"].toArray();
        for (int i=0;i<seams.size();++i) { auto row=seams[i].toObject(); row.remove("id"); row.remove("name"); row.remove("enabled"); seams[i]=row; }
        legacy["seams"]=seams; QVERIFY(decode(legacy,decoded,error)); QVERIFY(decoded.seams[0].enabled); QVERIFY(!decoded.seams[0].id.isEmpty());
        Project second; QVERIFY(decode(legacy,second,error)); QCOMPARE(decoded.seams[0].id,second.seams[0].id);
        for (const auto &kind : {"empty-name","bad-enabled","duplicate-id"}) {
            auto invalid=encode(dressFormDemoProject()); auto rows=invalid["seams"].toArray(); auto row=rows[0].toObject();
            if (QString(kind)=="empty-name") row["name"]=" ";
            if (QString(kind)=="bad-enabled") row["enabled"]="false";
            if (QString(kind)=="duplicate-id") { auto other=rows[1].toObject(); other["id"]=row["id"]; rows[1]=other; }
            rows[0]=row; invalid["seams"]=rows; const auto unchanged=encode(decoded); QVERIFY(!decode(invalid,decoded,error)); QCOMPARE(encode(decoded),unchanged);
        }
        auto merged=mergeGeometry(project,project); QCOMPARE(merged.seams[0].id,seam.id); QCOMPARE(merged.seams[0].name,seam.name);
    }
    void seamEditorWorkflow()
    {
        if (!qEnvironmentVariableIsSet("SEAMLY3D_GPU_TESTS")) QSKIP("Native GPU test is opt-in.");
        QTemporaryDir temporary; QString error; const auto path=temporary.filePath("seams.sm3d");
        QVERIFY(save(path,dressFormDemoProject(),error)); Window window; QVERIFY(window.openProject(path)); window.show();
        QTRY_VERIFY_WITH_TIMEOUT(window.rendererReady(),10000);
        auto tabs=window.findChild<QTabWidget *>("inspectorTabs"); tabs->setCurrentIndex(2);
        auto list=window.findChild<QListWidget *>("seamList"); list->setCurrentRow(0); const auto id=list->currentItem()->data(Qt::UserRole).toString();
        auto name=window.findChild<QLineEdit *>("seamName"); QCOMPARE(name->text(),QString("Right side"));
        auto enabled=window.findChild<QCheckBox *>("seamEnabled");
        auto apply=window.findChild<QPushButton *>("applySeamButton"); auto revert=window.findChild<QPushButton *>("revertSeamButton");
        auto history=window.findChild<QUndoStack *>("assemblyHistory"); auto view=window.findChild<Viewport *>();
        QVERIFY(view->seamPreviewActive()); QCOMPARE(view->seamGuides().size(),2);
        name->setText("Side seam right"); enabled->setChecked(false); QVERIFY(apply->isEnabled()); QCOMPARE(history->count(),0);
        apply->click(); QCOMPARE(history->count(),1); QCOMPARE(list->currentItem()->data(Qt::UserRole).toString(),id);
        QVERIFY(list->currentItem()->text().contains("Paused")); QVERIFY(window.saveToPath(path));
        Project saved; QVERIFY(load(path,saved,error)); QCOMPARE(saved.seams[0].name,QString("Side seam right")); QVERIFY(!saved.seams[0].enabled);
        history->undo(); QVERIFY(enabled->isChecked()); QCOMPARE(name->text(),QString("Right side")); history->redo(); QVERIFY(!enabled->isChecked());
        // Invalid self-overlap remains a draft; neither save nor switching can commit it.
        auto a=window.findChild<QComboBox *>("piece0"); auto b=window.findChild<QComboBox *>("piece1"); b->setCurrentIndex(a->currentIndex());
        window.findChild<QDoubleSpinBox *>("start1")->setValue(window.findChild<QDoubleSpinBox *>("start0")->value());
        window.findChild<QDoubleSpinBox *>("end1")->setValue(window.findChild<QDoubleSpinBox *>("end0")->value());
        QVERIFY(!apply->isEnabled()); QVERIFY(revert->isEnabled()); QVERIFY(!window.saveToPath(path));
        tabs->setCurrentIndex(1); QCOMPARE(tabs->currentIndex(),2); list->setCurrentRow(1); QCOMPARE(list->currentRow(),0);
        Project unchanged; QVERIFY(load(path,unchanged,error)); QCOMPARE(encode(unchanged),encode(saved));
        revert->click(); QVERIFY(!revert->isEnabled()); QVERIFY(!enabled->isChecked());
        // Valid drafts apply when selecting another seam, preserving its stable ID.
        name->setText("Renamed again"); list->setCurrentRow(1); QCOMPARE(name->text(),QString("Left side"));
        QVERIFY(window.saveToPath(path)); QVERIFY(load(path,saved,error)); QCOMPARE(saved.seams[0].id,id); QCOMPARE(saved.seams[0].name,QString("Renamed again"));
        view->setPieceVisible("back",false); QCOMPARE(view->seamGuides().size(),1); view->showAllPieces(); QCOMPARE(view->seamGuides().size(),2);
        tabs->setCurrentIndex(1); QVERIFY(!view->seamPreviewActive()); tabs->setCurrentIndex(2); QVERIFY(view->seamPreviewActive());
        window.findChild<QPushButton *>("newSeamButton")->click(); QVERIFY(name->text().isEmpty());
        QVERIFY(window.findChild<QPushButton *>("addSeamButton")->isVisible());
        window.setWindowModified(false); window.close();
    }
    void workspaceLayoutCapture()
    {
        if (!qEnvironmentVariableIsSet("SEAMLY3D_GPU_TESTS")) QSKIP("Native GPU test is opt-in.");
        const auto directory=qEnvironmentVariable("SEAMLY3D_UI_CAPTURE");
        QTemporaryDir temporary; QString error; const auto path=temporary.filePath(QStringLiteral("workspace.sm3d"));
        auto project=dressFormDemoProject();
        QVERIFY(save(path,project,error)); Window window; QVERIFY(window.openProject(path)); window.show();
        QTRY_VERIFY_WITH_TIMEOUT(window.rendererReady(),10000);
        auto tabs=window.findChild<QTabWidget *>(QStringLiteral("inspectorTabs")); QVERIFY(tabs);
        auto view=window.findChild<Viewport *>(); QVERIFY(view);
        view->selectPiece(QStringLiteral("front")); QCOMPARE(tabs->currentIndex(),1);
        auto selector=window.findChild<QComboBox *>(QStringLiteral("placementPiece")); QVERIFY(selector);
        QCOMPARE(selector->currentData().toString(),QStringLiteral("front"));
        tabs->setCurrentIndex(2); view->selectPiece(QStringLiteral("back")); QCOMPARE(tabs->currentIndex(),1);
        QCOMPARE(selector->currentData().toString(),QStringLiteral("back"));
        if (!directory.isEmpty())
        {
            QDir().mkpath(directory);
            auto x=window.findChild<QDoubleSpinBox *>("pose0"); x->setValue(x->value()+1);
            window.advance(300);
            auto capture=[&](const QString &name,int width,int height,int tab) {
                window.resize(width,height); tabs->setCurrentIndex(tab);
                if (tab==2 && window.findChild<QListWidget *>("seamList")->currentRow()<0) window.findChild<QListWidget *>("seamList")->setCurrentRow(0);
                QTest::qWait(80);
                return window.render(QDir(directory).filePath(name));
            };
#if QT_VERSION >= QT_VERSION_CHECK(6,8,0)
            QGuiApplication::styleHints()->setColorScheme(Qt::ColorScheme::Dark);
#endif
            QVERIFY(capture(QStringLiteral("desktop-form-dark.png"),1440,900,0));
            QVERIFY(capture(QStringLiteral("desktop-pieces-dark.png"),1440,900,1));
            QVERIFY(capture(QStringLiteral("desktop-seams-dark.png"),1440,900,2));
            QVERIFY(capture(QStringLiteral("compact-seams-dark.png"),1100,760,2));
            auto seamName=window.findChild<QLineEdit *>("seamName"); seamName->setText("Right side lining");
            auto seamEnabled=window.findChild<QCheckBox *>("seamEnabled"); seamEnabled->setChecked(false);
            QVERIFY(capture(QStringLiteral("desktop-seam-edit-dark.png"),1440,900,2));
            window.findChild<QPushButton *>("revertSeamButton")->click();
            view->selectPiece(QStringLiteral("front")); view->setToolMode(1);
            QVERIFY(capture(QStringLiteral("desktop-move-dark.png"),1440,900,1));
            view->setToolMode(2);
            QVERIFY(capture(QStringLiteral("desktop-rotate-dark.png"),1440,900,1));
            view->setToolMode(0);
            view->isolatePiece("front");
            QVERIFY(capture(QStringLiteral("desktop-isolated-dark.png"),1440,900,1));
            view->showAllPieces();
#if QT_VERSION >= QT_VERSION_CHECK(6,8,0)
            QGuiApplication::styleHints()->setColorScheme(Qt::ColorScheme::Light);
#endif
            QVERIFY(capture(QStringLiteral("desktop-form-light.png"),1440,900,0));
            QVERIFY(capture(QStringLiteral("compact-pieces-light.png"),1100,760,1));
            view->setView(1); view->setToolMode(1);
            QVERIFY(capture(QStringLiteral("compact-front-light.png"),1100,760,1));
            view->setToolMode(0); view->setView(0); view->setPieceTransparent("front",true);
            QVERIFY(capture(QStringLiteral("desktop-inspection-light.png"),1440,900,1));
            view->setPieceTransparent("front",false); tabs->setCurrentIndex(2); seamEnabled->setChecked(false);
            window.findChild<QPushButton *>("applySeamButton")->click(); window.advance(300);
            QVERIFY(capture(QStringLiteral("desktop-seam-paused-light.png"),1440,900,2));
            auto sideA=window.findChild<QComboBox *>("piece0"),sideB=window.findChild<QComboBox *>("piece1"); sideB->setCurrentIndex(sideA->currentIndex());
            window.findChild<QDoubleSpinBox *>("start1")->setValue(window.findChild<QDoubleSpinBox *>("start0")->value());
            window.findChild<QDoubleSpinBox *>("end1")->setValue(window.findChild<QDoubleSpinBox *>("end0")->value());
            QVERIFY(capture(QStringLiteral("compact-seam-invalid-light.png"),1100,760,2));
            window.findChild<QPushButton *>("revertSeamButton")->click();
            seamEnabled->setChecked(true); window.findChild<QPushButton *>("applySeamButton")->click();
            tabs->setCurrentIndex(3); auto mode=window.findChild<QComboBox *>("solverMode");
            mode->setCurrentIndex(1); QVERIFY(QMetaObject::invokeMethod(mode,"activated",Q_ARG(int,1)));
            view->selectPiece("front"); window.findChild<QCheckBox *>("pinTop")->setChecked(true);
            view->selectPiece("back"); window.findChild<QCheckBox *>("pinTop")->setChecked(true);
            window.advance(60);
            QVERIFY2(!window.findChild<QLabel *>("solverInfo")->text().contains("unstable"),qPrintable(window.findChild<QLabel *>("solverInfo")->text()));
            QVERIFY(capture(QStringLiteral("desktop-cloth-light.png"),1440,900,3));
            QVERIFY(capture(QStringLiteral("compact-cloth-light.png"),1100,760,3));
#if QT_VERSION >= QT_VERSION_CHECK(6,8,0)
            QGuiApplication::styleHints()->setColorScheme(Qt::ColorScheme::Dark);
#endif
            QVERIFY(capture(QStringLiteral("desktop-cloth-dark.png"),1440,900,3));
            QVERIFY(capture(QStringLiteral("compact-cloth-dark.png"),1100,760,3));
            tabs->setCurrentIndex(2); window.findChild<QDoubleSpinBox *>("start0")->setValue(21.123456);
            QVERIFY(capture(QStringLiteral("desktop-precise-seam-dark.png"),1440,900,2));
            window.findChild<QPushButton *>("revertSeamButton")->click();
#if QT_VERSION >= QT_VERSION_CHECK(6,8,0)
            QGuiApplication::styleHints()->setColorScheme(Qt::ColorScheme::Unknown);
#endif
        }
        window.setWindowModified(false); window.close();
    }
    void preciseSeamsAndAdaptiveMesh()
    {
        auto project=demoProject(); project.seams[0].startA=13.123456; project.seams[0].endA=47.987654;
        const auto &piece=project.pieces[0]; const double expectedLength=seamLength(piece,project.seams[0].startA,project.seams[0].endA);
        QVector<QVector3D> endpoints; int previous=0; QString error;
        for (double size : {50.,25.,10.})
        {
            project.meshSize=size; Assembly assembly; QVERIFY2(assembly.reset(project,error),qPrintable(error)); const auto &mesh=assembly.meshes()[0];
            QVERIFY(mesh.vertices.size()>previous); previous=int(mesh.vertices.size());
            endpoints<<boundaryPoint(mesh,project.seams[0].startA); QCOMPARE(seamLength(piece,project.seams[0].startA,project.seams[0].endA),expectedLength);
            for (const auto &e : mesh.edges) QVERIFY(e.length<=size*1.0001);
            QMap<QPair<int,int>,int> edges;
            for (const auto &t : mesh.triangles) for (int k=0;k<3;++k) { const int a=t[k],b=t[(k+1)%3]; ++edges[qMakePair(qMin(a,b),qMax(a,b))]; }
            int border=0; for (auto i=edges.cbegin();i!=edges.cend();++i) { QVERIFY(i.value()==1 || i.value()==2); if (i.value()==1) ++border; }
            QCOMPARE(border,mesh.boundary.size());
            for (int i=0;i<mesh.parameters.size();++i)
            {
                const auto current=contourPoint(piece.outline,mesh.parameters[i]),origin=contourPoint(piece.outline,0);
                const auto offset=mesh.rest[mesh.boundary[i]]-mesh.rest[mesh.boundary[0]];
                QVERIFY((QVector3D(float(current.x()-origin.x()),float(current.y()-origin.y()),0)-offset).length()<0.001f);
            }
        }
        for (const auto &point : endpoints) QVERIFY((point-endpoints.first()).length()<0.001f);
        Project decoded; QVERIFY(decode(encode(project),decoded,error)); QCOMPARE(decoded.seams[0].startA,project.seams[0].startA);
        auto legacy=legacyProject(demoProject(),3); QVERIFY(decode(legacy,decoded,error)); QCOMPARE(decoded.seams[0].startA,16.0);
        auto bad=encode(project); bad["meshSize"]=0; const auto baseline=encode(decoded); QVERIFY(!decode(bad,decoded,error)); QCOMPARE(encode(decoded),baseline);
        Assembly assembly; QVERIFY(assembly.reset(project,error)); const auto before=assembly.meshes()[0].vertices;
        auto excessive=project; excessive.pieces[0].outline={{0,0},{100000,0},{100000,100000},{0,100000}}; excessive.meshSize=5;
        QVERIFY(!assembly.reset(excessive,error)); QCOMPARE(assembly.meshes()[0].vertices,before);
        auto merged=mergeGeometry(project,project); QCOMPARE(merged.meshSize,project.meshSize);
        auto changed=project; changed.pieces[0].outline[0]+=QPointF(1,0); QVERIFY(mergeGeometry(project,changed).seams.isEmpty());
    }
    void clothFreeFallAndMass()
    {
        Project project; Piece p; p.id="fall"; p.outline={{-50,-50},{50,-50},{50,50},{-50,50}}; project.pieces={p};
        project.simulation.physical=true; project.simulation.damping=0; project.meshSize=25;
        Assembly assembly; QString error; QVERIFY(assembly.reset(project,error)); const auto before=assembly.meshes()[0].vertices;
        double mass=0; for (float inverse : assembly.meshes()[0].inverseMass) mass+=1/inverse;
        QVERIFY(std::abs(mass-0.002)<1e-8); // 0.01 m² at 200 g/m².
        assembly.step(6); QVERIFY(std::abs(assembly.simulationTime()-0.1)<1e-8);
        for (int i=0;i<before.size();++i)
        { const auto delta=assembly.meshes()[0].vertices[i]-before[i]; QVERIFY(std::abs(delta.y()+51.09375)<0.02); QVERIFY(std::abs(delta.x())<0.01); }
        QVERIFY(assembly.maximumSpeed()>970 && assembly.maximumSpeed()<990);
        QVERIFY(assembly.meanStrain()<0.001);
        project.simulation.density=400; Assembly heavy; QVERIFY(heavy.reset(project,error)); heavy.step(6);
        for (int i=0;i<before.size();++i) QVERIFY((heavy.meshes()[0].vertices[i]-assembly.meshes()[0].vertices[i]).length()<0.02);
        auto json=encode(project); auto settings=json["simulation"].toObject(); settings["density"]=-1; json["simulation"]=settings; Project decoded=project;
        QVERIFY(!decode(json,decoded,error)); QCOMPARE(encode(decoded),encode(project));
    }
    void clothPinsFloorAndReset()
    {
        Project project; Piece p; p.id="pinned"; p.outline={{-50,-50},{50,-50},{50,50},{-50,50}}; p.rotation={90,0,0}; p.pinTop=true;
        project.pieces={p}; project.simulation.physical=true; project.meshSize=25;
        Assembly assembly; QString error; QVERIFY(assembly.reset(project,error)); const auto initial=assembly.meshes()[0].vertices; assembly.step(60);
        int fixed=0; float movement=0; const auto &mesh=assembly.meshes()[0];
        for (int i=0;i<initial.size();++i) { if (mesh.inverseMass[i]==0) { ++fixed; QCOMPARE(mesh.vertices[i],initial[i]); } else movement=qMax(movement,(mesh.vertices[i]-initial[i]).length()); }
        QVERIFY(fixed>=2); QVERIFY(movement>10); QVERIFY(assembly.meanStrain()<0.15);
        QVERIFY(assembly.reset(project,error)); QCOMPARE(assembly.simulationTime(),0.0); QCOMPARE(assembly.meshes()[0].vertices,initial);
        project.pieces[0].pinTop=false; QVERIFY(assembly.reset(project,error)); for (int i=0;i<6;++i) assembly.step(100);
        for (const auto &point : assembly.meshes()[0].vertices) QVERIFY(point.y()>=-499.01f);
        QVERIFY(assembly.simulationStopped()); QVERIFY(assembly.simulationStatus().contains("rest"));
    }
    void clothBodyContactAndSeams_data()
    {
        QTest::addColumn<double>("edgeSize"); QTest::newRow("25-mm")<<25.0; QTest::newRow("50-mm")<<50.0; QTest::newRow("rounded-corners")<<24.0;
    }
    void clothBodyContactAndSeams()
    {
        QFETCH(double,edgeSize);
        auto project=dressFormDemoProject(); project.meshSize=edgeSize==24 ? 25 : edgeSize; project.simulation.physical=true;
        if (edgeSize==24) { project.pieces[1].translation.setX(1); for (auto &seam : project.seams) { seam.startA=std::round(seam.startA/BoundarySamples*1e8)/1e8*BoundarySamples; seam.endA=std::round(seam.endA/BoundarySamples*1e8)/1e8*BoundarySamples; seam.startB=std::round(seam.startB/BoundarySamples*1e8)/1e8*BoundarySamples; seam.endB=std::round(seam.endB/BoundarySamples*1e8)/1e8*BoundarySamples; } }
        for (auto &piece : project.pieces) piece.pinTop=true;
        Assembly assembly; QString error; QVERIFY2(assembly.reset(project,error),qPrintable(error)); const double before=assembly.seamError();
        const auto profile=dressFormProfile(project.dressForm); assembly.step(60);
        QVERIFY2(!assembly.simulationStatus().contains("unstable"),qPrintable(assembly.simulationStatus()));
        qInfo()<<"Cloth benchmark"<<assembly.meshes()[0].vertices.size()<<"vertices per panel; gap"<<assembly.seamError()<<"strain"<<assembly.meanStrain();
        QVERIFY(assembly.seamError()<before); QVERIFY(assembly.meanStrain()<0.2);
        for (const auto &mesh : assembly.meshes()) for (int i=0;i<mesh.vertices.size();++i)
            if (mesh.inverseMass[i]>0) QVERIFY((outsideDressForm(profile,mesh.vertices[i],float(project.dressForm.clearance))-mesh.vertices[i]).length()<0.05f);
        Project decoded; QVERIFY(decode(encode(project),decoded,error)); QVERIFY(decoded.simulation.physical); QVERIFY(decoded.pieces[0].pinTop);
    }
    void clothControlsWorkflow()
    {
        if (!qEnvironmentVariableIsSet("SEAMLY3D_GPU_TESTS")) QSKIP("Native GPU test is opt-in.");
        QTemporaryDir directory; QString error; const auto path=directory.filePath("cloth.sm3d");
        auto project=demoProject(); QVERIFY(save(path,project,error)); Window window; QVERIFY(window.openProject(path)); window.show();
        QTRY_VERIFY_WITH_TIMEOUT(window.rendererReady(),10000);
        auto tabs=window.findChild<QTabWidget *>("inspectorTabs"); tabs->setCurrentIndex(3);
        auto size=window.findChild<QDoubleSpinBox *>("meshSize"); auto history=window.findChild<QUndoStack *>("assemblyHistory");
        size->setValue(15); QVERIFY(QMetaObject::invokeMethod(size,"editingFinished")); QCOMPARE(history->count(),1);
        QVERIFY(window.saveToPath(path)); Project saved; QVERIFY(load(path,saved,error)); QCOMPARE(saved.meshSize,15.0);
        history->undo(); QCOMPARE(size->value(),25.0); history->redo(); QCOMPARE(size->value(),15.0);
        auto mode=window.findChild<QComboBox *>("solverMode"); mode->setCurrentIndex(1); QVERIFY(QMetaObject::invokeMethod(mode,"activated",Q_ARG(int,1)));
        QVERIFY(window.findChild<QDoubleSpinBox *>("clothParameter0")->isEnabled()); window.advance(6);
        QVERIFY(window.findChild<QLabel *>("solverInfo")->text().contains("0.10"));
        window.findChild<Viewport *>()->selectPiece("front"); auto pin=window.findChild<QCheckBox *>("pinTop"); pin->setChecked(true);
        QVERIFY(window.saveToPath(path)); QVERIFY(load(path,saved,error)); QVERIFY(saved.pieces[0].pinTop); QVERIFY(saved.simulation.physical);
        window.setWindowModified(false); window.close();
    }
    void concaveContour()
    {
        Project project; Piece p; p.id="concave"; p.outline={{0,0},{100,0},{100,40},{40,40},{40,100},{0,100}}; project.pieces << p;
        Assembly assembly; QString error; QVERIFY2(assembly.reset(project,error),qPrintable(error));
        QVERIFY(!assembly.meshes()[0].triangles.isEmpty());
    }
};
QTEST_MAIN(Seamly3DTest)
#include "tst_seamly3d.moc"
