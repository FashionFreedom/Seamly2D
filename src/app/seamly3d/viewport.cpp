// SPDX-License-Identifier: GPL-3.0-or-later
#include "viewport.h"
#include "scenegeometry.h"
#include "dressform.h"
#include <QQuickWidget>
#include <QQuickWindow>
#include <QSGRendererInterface>
#include <QQmlContext>
#include <QQmlEngine>
#include <QQmlError>
#include <QLabel>
#include <QVBoxLayout>
#include <QResizeEvent>
#include <QMouseEvent>
#include <QLineF>
#include <QWheelEvent>
#include <QPainter>
#include <QPainterPath>
#include <QMatrix4x4>
#include <algorithm>
#include <cmath>

namespace Seamly3D
{
Viewport::Viewport(const Project *project, const Assembly *assembly, QWidget *parent)
    : QWidget(parent),m_project(project),m_assembly(assembly)
{
    setToolTip(tr("Click to select. Drag to orbit. Right-drag to pan. Scroll to zoom. F fits the view."));
    setMinimumSize(400,350); setObjectName(QStringLiteral("assemblyViewport"));
    setAccessibleName(tr("Assembly 3D view"));
    auto layout=new QVBoxLayout(this); layout->setContentsMargins(0,0,0,0);
    m_quick=new QQuickWidget(this); m_quick->setResizeMode(QQuickWidget::SizeRootObjectToView);
    m_quick->setClearColor(QColor(24,31,40)); layout->addWidget(m_quick);
    m_errorLabel=new QLabel(this); m_errorLabel->setWordWrap(true); m_errorLabel->hide(); layout->addWidget(m_errorLabel);
    static const int scenePieceType=qmlRegisterUncreatableType<ScenePiece>("Seamly3D",1,0,"ScenePiece",QStringLiteral("Created by the assembly viewer"));
    Q_UNUSED(scenePieceType)
    m_formScene=new ScenePiece(QStringLiteral("__dress_form"),this);
    m_quick->rootContext()->setContextProperty(QStringLiteral("assemblyView"),this);
    auto showError=[this](const QString &message) {
        m_error=message; m_errorLabel->setText(tr("The 3D renderer could not start: %1").arg(message));
        m_errorLabel->show(); emit backendChanged();
    };
    connect(m_quick,&QQuickWidget::sceneGraphError,this,[showError](QQuickWindow::SceneGraphError,const QString &message) { showError(message); });
    connect(m_quick,&QQuickWidget::statusChanged,this,[this,showError](QQuickWidget::Status status) {
        if (status!=QQuickWidget::Error) return;
        QStringList messages; for (const auto &error : m_quick->errors()) messages << error.toString();
        showError(messages.join(QLatin1Char('\n')));
    });
    connect(m_quick->quickWindow(),&QQuickWindow::afterRendering,this,[this,showError]() {
        const auto api=m_quick->quickWindow()->rendererInterface()->graphicsApi();
        if (api==QSGRendererInterface::Software || api==QSGRendererInterface::Null || api==QSGRendererInterface::Unknown)
        {
            if (m_error.isEmpty()) showError(tr("Qt Quick 3D requires a hardware graphics backend."));
            return;
        }
        if (!m_frameRendered) { m_frameRendered=true; emit backendChanged(); }
    },Qt::QueuedConnection);
    connect(m_quick->quickWindow(),&QQuickWindow::sceneGraphInvalidated,this,[this]() {
        m_frameRendered=false; emit backendChanged();
    },Qt::QueuedConnection);
    m_quick->setSource(QUrl(QStringLiteral("qrc:/seamly3d/AssemblyView.qml")));
}
Viewport::~Viewport()
{
    // Destroy QML models before the C++ geometry they reference.
    delete m_quick;
}
QVariantList Viewport::pieces() const
{
    QVariantList result; for (auto piece : m_pieces) result << QVariant::fromValue(piece); return result;
}
void Viewport::synchronize()
{
    if (m_physicalFloor!=m_project->simulation.physical) { m_physicalFloor=m_project->simulation.physical; emit cameraChanged(); }
    if (!m_formInitialized || m_formSettings!=m_project->dressForm)
    {
        m_formSettings=m_project->dressForm; m_formInitialized=true;
        if (m_formSettings.enabled)
        {
            Piece form; form.name=tr("Dress form");
            m_formScene->synchronize(dressFormMesh(m_formSettings),form,{},0);
        }
        emit dressFormChanged(); emit cameraChanged();
    }
    const auto &meshes=m_assembly->meshes();
    bool replaced=m_pieces.size()!=meshes.size();
    if (!replaced) for (int i=0; i<meshes.size(); ++i) if (m_pieces[i]->pieceId()!=meshes[i].id) { replaced=true; break; }
    if (replaced)
    {
        const auto old=m_pieces; m_pieces.clear();
        for (const auto &mesh : meshes) m_pieces << new ScenePiece(mesh.id,this);
        for (int i=0; i<meshes.size(); ++i) m_pieces[i]->synchronize(meshes[i],m_project->pieces[i],m_project->seams,i);
        emit piecesChanged();
        for (auto piece : old) piece->deleteLater();
    }
    else for (int i=0; i<meshes.size(); ++i) m_pieces[i]->synchronize(meshes[i],m_project->pieces[i],m_project->seams,i);
    bool selected=false; for (auto piece : m_pieces) if (piece->pieceId()==m_selectedId) selected=true;
    if (!selected && !m_selectedId.isEmpty()) { m_selectedId.clear(); emit selectionChanged(); }
    else if (selected) emit selectionChanged();
    for (auto piece : m_pieces) piece->setInspection(pieceVisible(piece->pieceId()),pieceTransparent(piece->pieceId()));
    if (m_seamPreview) emit seamPreviewChanged();
    m_quick->update();
}
void Viewport::fit()
{
    endTransform(true);
    QVector3D minimum(1e9f,1e9f,1e9f),maximum(-1e9f,-1e9f,-1e9f);
    bool any=false;
    for (const auto &mesh : m_assembly->meshes()) for (const auto &p : mesh.vertices)
    {
        any=true;
        for (int k=0; k<3; ++k) { minimum[k]=qMin(minimum[k],p[k]); maximum[k]=qMax(maximum[k],p[k]); }
    }
    if (m_project->dressForm.enabled)
    {
        any=true;
        for (int k=0; k<3; ++k)
        {
            minimum[k]=qMin(minimum[k],m_formScene->surface()->boundsMin()[k]);
            maximum[k]=qMax(maximum[k],m_formScene->surface()->boundsMax()[k]);
        }
        minimum.setY(qMin(minimum.y(),dressFormFloor()));
    }
    m_center=any ? (minimum+maximum)*0.5f : QVector3D();
    m_radius=any ? qMax(1.f,(maximum-minimum).length()*0.5f) : 100;
    m_zoom=1; m_pan=QPointF(); emit cameraChanged();
}
float Viewport::magnification() const { return float(qMin(width(),height()))*0.4f*m_zoom/m_radius; }
QVector3D Viewport::cameraPan() const { return QVector3D(float(m_pan.x())/magnification(),-float(m_pan.y())/magnification(),0); }
void Viewport::resizeEvent(QResizeEvent *event) { QWidget::resizeEvent(event); emit cameraChanged(); }
void Viewport::setWireframe(bool enabled) { if (enabled!=m_wireframe) { m_wireframe=enabled; emit wireframeChanged(); } }
QString Viewport::selectedName() const
{
    for (auto piece : m_pieces) if (piece->pieceId()==m_selectedId) return piece->name();
    return QString();
}
void Viewport::selectPiece(const QString &id)
{
    if (!id.isEmpty())
    {
        bool found=false; for (auto piece : m_pieces) if (piece->pieceId()==id) found=true;
        if (!found) return;
    }
    if (m_selectedId==id) { if (!id.isEmpty()) emit pieceSelected(id); return; }
    endTransform(true);
    m_selectedId=id; emit selectionChanged(); if (!id.isEmpty()) emit pieceSelected(id);
}
void Viewport::orbit(float dx,float dy) { m_yaw+=dx*0.5f; m_pitch=qBound(-89.f,m_pitch+dy*0.5f,89.f); emit cameraChanged(); emit viewChanged(-1); }
void Viewport::pan(float dx,float dy) { m_pan+=QPointF(-dx,-dy); emit cameraChanged(); }
void Viewport::zoom(float delta) { m_zoom=qBound(0.1f,m_zoom*float(std::exp(delta/1200.0)),15.f); emit cameraChanged(); }
void Viewport::setToolMode(int mode)
{
    if (mode<0 || mode>2 || mode==m_toolMode) return;
    endTransform(true); m_toolMode=mode; emit toolModeChanged();
}
QVariantList Viewport::seamGuides() const
{
    QVariantList guides;
    if (!m_seamPreview) return guides;
    for (int side=0; side<2; ++side)
    {
        const auto &seam=*m_seamPreview; const auto id=side==0 ? seam.a : seam.b;
        if (!pieceVisible(id)) continue;
        for (const auto &mesh : m_assembly->meshes()) if (mesh.id==id)
        {
            QVariantList points; const double start=side==0 ? seam.startA : seam.startB,end=side==0 ? seam.endA : seam.endB;
            for (const auto &point : seamPath(mesh,start,end)) points<<QVariant::fromValue(point);
            guides << QVariantMap{{QStringLiteral("side"),side},{QStringLiteral("points"),points}};
        }
    }
    return guides;
}
void Viewport::setSeamPreview(const Seam *seam)
{
    m_seamPreview=seam ? std::optional<Seam>(*seam) : std::nullopt;
    emit seamPreviewChanged(); synchronize();
}
void Viewport::refreshInspection()
{
    for (auto piece : m_pieces) piece->setInspection(pieceVisible(piece->pieceId()),pieceTransparent(piece->pieceId()));
    emit selectionChanged(); emit inspectionChanged(); m_quick->update();
}
void Viewport::setPieceVisible(const QString &id,bool visible)
{
    bool exists=false; for (auto piece : m_pieces) if (piece->pieceId()==id) exists=true;
    if (!exists || pieceVisible(id)==visible) return;
    if (!visible && id==m_selectedId) endTransform(true);
    if (visible) m_hidden.remove(id); else m_hidden.insert(id);
    refreshInspection();
}
void Viewport::setPieceTransparent(const QString &id,bool transparent)
{
    bool exists=false; for (auto piece : m_pieces) if (piece->pieceId()==id) exists=true;
    if (!exists || pieceTransparent(id)==transparent) return;
    if (transparent) m_transparent.insert(id); else m_transparent.remove(id);
    refreshInspection();
}
void Viewport::isolatePiece(const QString &id)
{
    bool exists=false; for (auto piece : m_pieces) if (piece->pieceId()==id) exists=true;
    if (!exists) return;
    endTransform(true); m_hidden.clear();
    for (auto piece : m_pieces) if (piece->pieceId()!=id) m_hidden.insert(piece->pieceId());
    refreshInspection();
}
void Viewport::showAllPieces() { m_hidden.clear(); refreshInspection(); }
void Viewport::resetInspection() { m_hidden.clear(); m_transparent.clear(); refreshInspection(); }
void Viewport::setView(int view)
{
    if (view<0 || view>5) return;
    endTransform(true);
    const float yaw[]={-25,0,180,90,-90,0},pitch[]={-20,0,0,0,0,-89};
    m_yaw=yaw[view]; m_pitch=pitch[view]; emit cameraChanged(); emit viewChanged(view);
}
QVector3D Viewport::placementOrigin() const
{
    for (const auto &piece : m_project->pieces) if (piece.id==m_selectedId) return piece.translation;
    return {};
}
QVector3D Viewport::handleAxis(int axis) const
{
    if (axis<0 || axis>2) return {};
    QVector3D direction; direction[axis]=1;
    if (m_toolMode==2)
        for (const auto &piece : m_project->pieces) if (piece.id==m_selectedId)
        {
            // Axes for the stored Rz * Ry * Rx Euler placement, matching numeric edits.
            QMatrix4x4 rotation;
            if (axis<2) rotation.rotate(piece.rotation.z(),0,0,1);
            if (axis==0) rotation.rotate(piece.rotation.y(),0,1,0);
            return rotation.mapVector(direction);
        }
    return direction;
}
bool Viewport::beginTransform()
{
    if (!m_toolMode || !selectedVisible() || m_transforming) return false;
    m_transforming=true; emit transformStarted(m_selectedId); return true;
}
void Viewport::transformBy(int axis,float total)
{
    if (m_transforming && axis>=0 && axis<3 && std::isfinite(total))
        emit transformRequested(m_selectedId,axis,m_toolMode==2,total);
}
void Viewport::endTransform(bool cancel)
{
    if (m_transforming) { m_transforming=false; emit transformFinished(cancel); }
}
QString Viewport::backend() const
{
    if (!m_error.isEmpty()) return tr("Renderer unavailable");
    if (!m_frameRendered) return tr("Starting 3D renderer…");
    switch (m_quick->quickWindow()->rendererInterface()->graphicsApi())
    {
    case QSGRendererInterface::Metal: return QStringLiteral("Metal");
    case QSGRendererInterface::Vulkan: return QStringLiteral("Vulkan");
    case QSGRendererInterface::Direct3D11: return QStringLiteral("Direct3D 11");
    case QSGRendererInterface::OpenGL: return QStringLiteral("OpenGL");
    default: return QStringLiteral("Qt RHI");
    }
}
QImage Viewport::capture()
{
    if (!m_error.isEmpty() || !m_frameRendered) return {};
    return m_quick->grabFramebuffer();
}
QQuick3DGeometry *Viewport::dressFormGeometry() const { return m_formScene->surface(); }
float Viewport::dressFormTop() const { return float(Seamly3D::dressFormTop(m_project->dressForm)); }
float Viewport::dressFormBottom() const { return float(Seamly3D::dressFormBottom(m_project->dressForm)); }
float Viewport::dressFormFloor() const { return float(Seamly3D::dressFormFloor(m_project->dressForm)); }
QVector3D Viewport::dressFormNeck() const
{
    const auto radius=dressFormRadii(m_project->dressForm,dressFormTop());
    return QVector3D(radius.x(),0,radius.y());
}
float Viewport::floorHeight() const { return m_project->simulation.physical ? (dressFormEnabled() ? dressFormFloor() : -500.f) : (dressFormEnabled() ? dressFormFloor()-5 : m_center.y()-m_radius); }
ContourView::ContourView(QWidget *parent) : QWidget(parent)
{
    setMinimumSize(120,170);
    setToolTip(tr("Click the start, then the end of the sewing arc. The arc follows increasing perimeter percentages and wraps through 0. Equal endpoints select the full loop."));
    setAccessibleName(tr("Sewing contour endpoint picker"));
}
void ContourView::setPiece(const Piece *piece, double start, double end) { m_piece=piece; m_start=start; m_end=end; update(); }
QVector<QPointF> ContourView::screenPoints() const
{
    if (!m_piece) return {};
    const auto points=m_piece->outline;
    double left=1e9,right=-1e9,bottom=1e9,top=-1e9;
    for (const auto &p : points) { left=qMin(left,p.x()); right=qMax(right,p.x()); bottom=qMin(bottom,p.y()); top=qMax(top,p.y()); }
    const double scale=qMin((width()-44)/qMax(1.0,right-left),(height()-58)/qMax(1.0,top-bottom));
    QVector<QPointF> output;
    for (const auto &p : points) output << QPointF(width()/2.0+(p.x()-(left+right)/2)*scale,height()/2.0-(p.y()-(bottom+top)/2)*scale-6);
    return output;
}
void ContourView::paintEvent(QPaintEvent *)
{
    QPainter painter(this); painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(rect(),palette().base());
    const auto points=screenPoints(); if (points.isEmpty()) return;
    painter.setPen(QPen(palette().mid().color(),1)); painter.setBrush(QColor(80,155,180,30));
    painter.drawPolygon(QPolygonF(points));
    const bool dark=palette().base().color().lightness()<128;
    const QColor accent=m_side==0 ? QColor(dark ? "#70c9ed" : "#1d7897") : QColor(dark ? "#f3a48d" : "#a34831");
    painter.setPen(QPen(accent,3,m_side==0 ? Qt::SolidLine : Qt::DashLine));
    const double total=perimeter(points),length=arcSteps(m_start,m_end);
    QVector<double> offsets{0,length}; double before=0;
    for (int i=0;i<points.size();++i)
    {
        const double offset=std::fmod(before/total*BoundarySamples-m_start+BoundarySamples,BoundarySamples);
        if (offset>1e-9 && offset<length-1e-9) offsets<<offset;
        before+=QLineF(points[i],points[(i+1)%points.size()]).length();
    }
    std::sort(offsets.begin(),offsets.end());
    QPainterPath path; path.moveTo(contourPoint(points,m_start/BoundarySamples));
    for (double d : offsets) path.lineTo(contourPoint(points,(m_start+d)/BoundarySamples)); painter.drawPath(path);
    for (int i=0;i<4;++i)
    {
        const auto point=contourPoint(points,i/4.0); painter.setPen(palette().text().color()); painter.setBrush(palette().base());
        painter.drawEllipse(point,2,2); painter.drawText(point+QPointF(5,-5),QString::number(i*25)+QStringLiteral("%"));
    }
    for (int i=0;i<2;++i)
    {
        const auto point=contourPoint(points,(i ? m_end : m_start)/BoundarySamples);
        painter.setPen(palette().text().color()); painter.setBrush(i ? QColor(221,108,76) : QColor(43,162,118)); painter.drawEllipse(point,5,5);
    }
    painter.setPen(palette().text().color());
    painter.drawText(QRect(5,height()-24,width()-10,20),Qt::AlignCenter,m_pickEnd ? tr("Click arc end") : tr("Click arc start"));
}
void ContourView::mousePressEvent(QMouseEvent *event)
{
    if (event->button()!=Qt::LeftButton) return;
    const auto points=screenPoints(); if (points.isEmpty()) return;
    double best=0,distance=1e9,before=0; const double total=perimeter(points);
    for (int i=0;i<points.size();++i)
    {
        const auto a=points[i],b=points[(i+1)%points.size()],delta=b-a;
        const double length2=QPointF::dotProduct(delta,delta),length=std::sqrt(length2);
        const double t=length2>0 ? qBound(0.0,QPointF::dotProduct(event->position()-a,delta)/length2,1.0) : 0;
        const double d=QLineF(a+delta*t,event->position()).length();
        if (d<distance) { distance=d; best=std::fmod((before+t*length)/total,1.0)*BoundarySamples; }
        before+=length;
    }
    if (distance>24) return;
    if (endpointPicked) endpointPicked(best,m_pickEnd);
    m_pickEnd=!m_pickEnd; update();
}
}
