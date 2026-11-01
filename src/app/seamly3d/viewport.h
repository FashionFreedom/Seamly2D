// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef SEAMLY3D_VIEWPORT_H
#define SEAMLY3D_VIEWPORT_H
#include "model.h"
#include <QWidget>
#include <functional>
#include <QVariantList>
#include <QImage>
#include <QSet>
#include <optional>
#include <QQuick3DGeometry>
class QQuickWidget;
class QLabel;

namespace Seamly3D
{
class ScenePiece;
class Viewport : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QVariantList pieces READ pieces NOTIFY piecesChanged)
    Q_PROPERTY(QVector3D sceneCenter READ sceneCenter NOTIFY cameraChanged)
    Q_PROPERTY(float sceneRadius READ sceneRadius NOTIFY cameraChanged)
    Q_PROPERTY(float magnification READ magnification NOTIFY cameraChanged)
    Q_PROPERTY(float yaw READ yaw NOTIFY cameraChanged)
    Q_PROPERTY(float pitch READ pitch NOTIFY cameraChanged)
    Q_PROPERTY(QVector3D cameraPan READ cameraPan NOTIFY cameraChanged)
    Q_PROPERTY(bool wireframe READ wireframe WRITE setWireframe NOTIFY wireframeChanged)
    Q_PROPERTY(QString selectedId READ selectedId NOTIFY selectionChanged)
    Q_PROPERTY(QString selectedName READ selectedName NOTIFY selectionChanged)
    Q_PROPERTY(QString backend READ backend NOTIFY backendChanged)
    Q_PROPERTY(QQuick3DGeometry *dressFormGeometry READ dressFormGeometry CONSTANT)
    Q_PROPERTY(bool dressFormEnabled READ dressFormEnabled NOTIFY dressFormChanged)
    Q_PROPERTY(bool dressFormStand READ dressFormStand NOTIFY dressFormChanged)
    Q_PROPERTY(float dressFormTop READ dressFormTop NOTIFY dressFormChanged)
    Q_PROPERTY(float dressFormBottom READ dressFormBottom NOTIFY dressFormChanged)
    Q_PROPERTY(float dressFormFloor READ dressFormFloor NOTIFY dressFormChanged)
    Q_PROPERTY(QVector3D dressFormNeck READ dressFormNeck NOTIFY dressFormChanged)
    Q_PROPERTY(float floorHeight READ floorHeight NOTIFY cameraChanged)
    Q_PROPERTY(int toolMode READ toolMode WRITE setToolMode NOTIFY toolModeChanged)
    Q_PROPERTY(QVector3D placementOrigin READ placementOrigin NOTIFY selectionChanged)
    Q_PROPERTY(bool selectedVisible READ selectedVisible NOTIFY selectionChanged)
    Q_PROPERTY(QString seamPreviewName READ seamPreviewName NOTIFY seamPreviewChanged)
    Q_PROPERTY(QVariantList seamGuides READ seamGuides NOTIFY seamPreviewChanged)
    Q_PROPERTY(bool seamPreviewActive READ seamPreviewActive NOTIFY seamPreviewChanged)
public:
    Viewport(const Project *project, const Assembly *assembly, QWidget *parent=nullptr);
    ~Viewport() override;
    Q_INVOKABLE void fit();
    void synchronize();
    bool wireframe() const { return m_wireframe; }
    void setWireframe(bool enabled);
    QVariantList pieces() const;
    QVector3D sceneCenter() const { return m_center; }
    float sceneRadius() const { return m_radius; }
    float magnification() const;
    float yaw() const { return m_yaw; }
    float pitch() const { return m_pitch; }
    QVector3D cameraPan() const;
    QString selectedId() const { return m_selectedId; }
    QString selectedName() const;
    QString backend() const;
    QString rendererError() const { return m_error; }
    bool rendererReady() const { return m_frameRendered && m_error.isEmpty(); }
    QImage capture();
    QQuick3DGeometry *dressFormGeometry() const;
    bool dressFormEnabled() const { return m_project->dressForm.enabled; }
    bool dressFormStand() const { return m_project->dressForm.stand; }
    float dressFormTop() const;
    float dressFormBottom() const;
    float dressFormFloor() const;
    QVector3D dressFormNeck() const;
    float floorHeight() const;
    Q_INVOKABLE void selectPiece(const QString &id);
    Q_INVOKABLE void orbit(float dx, float dy);
    Q_INVOKABLE void pan(float dx, float dy);
    Q_INVOKABLE void zoom(float delta);
    int toolMode() const { return m_toolMode; }
    void setToolMode(int mode);
    Q_INVOKABLE void setView(int view);
    QVector3D placementOrigin() const;
    Q_INVOKABLE QVector3D handleAxis(int axis) const;
    Q_INVOKABLE bool beginTransform();
    Q_INVOKABLE void transformBy(int axis, float total);
    Q_INVOKABLE void endTransform(bool cancel=false);
    bool pieceVisible(const QString &id) const { return !m_hidden.contains(id); }
    bool pieceTransparent(const QString &id) const { return m_transparent.contains(id); }
    bool selectedVisible() const { return !m_selectedId.isEmpty() && pieceVisible(m_selectedId); }
    void setPieceVisible(const QString &id,bool visible);
    void setPieceTransparent(const QString &id,bool transparent);
    void isolatePiece(const QString &id);
    void showAllPieces();
    void resetInspection();
    void setSeamPreview(const Seam *seam);
    QVariantList seamGuides() const;
    QString seamPreviewName() const { return m_seamPreview ? m_seamPreview->name : QString(); }
    bool seamPreviewActive() const { return m_seamPreview.has_value(); }
signals:
    void piecesChanged();
    void cameraChanged();
    void wireframeChanged();
    void selectionChanged();
    void pieceSelected(const QString &id);
    void backendChanged();
    void dressFormChanged();
    void toolModeChanged();
    void viewChanged(int view);
    void transformStarted(const QString &id);
    void transformRequested(const QString &id,int axis,bool rotation,float total);
    void transformFinished(bool cancel);
    void inspectionChanged();
    void seamPreviewChanged();
protected:
    void resizeEvent(QResizeEvent *) override;
private:
    const Project *m_project;
    const Assembly *m_assembly;
    ::QQuickWidget *m_quick;
    ::QLabel *m_errorLabel;
    QVector<ScenePiece *> m_pieces;
    ScenePiece *m_formScene;
    DressForm m_formSettings;
    bool m_formInitialized=false,m_physicalFloor=false;
    QString m_selectedId,m_error;
    bool m_wireframe=false,m_frameRendered=false;
    float m_yaw=-25, m_pitch=-20, m_zoom=1;
    QPointF m_pan;
    QVector3D m_center;
    float m_radius=100;
    int m_toolMode=0;
    bool m_transforming=false;
    QSet<QString> m_hidden,m_transparent;
    std::optional<Seam> m_seamPreview;
    void refreshInspection();
};
class ContourView : public QWidget
{
public:
    explicit ContourView(QWidget *parent=nullptr);
    void setPiece(const Piece *piece, double start, double end);
    void setSide(int side) { m_side=side; update(); }
    std::function<void(double,bool)> endpointPicked;
protected:
    void paintEvent(QPaintEvent *) override;
    void mousePressEvent(QMouseEvent *) override;
private:
    QVector<QPointF> screenPoints() const;
    const Piece *m_piece=nullptr;
    double m_start=0,m_end=16;
    bool m_pickEnd=false;
    int m_side=0;
};
}
#endif
