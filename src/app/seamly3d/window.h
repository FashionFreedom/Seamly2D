// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef SEAMLY3D_WINDOW_H
#define SEAMLY3D_WINDOW_H
#include "model.h"
#include <QMainWindow>
class QComboBox;
class QSpinBox;
class QDoubleSpinBox;
class QCheckBox;
class QLabel;
class QListWidget;
class QPushButton;
class QTimer;
class QGroupBox;
class QTabWidget;
class QUndoStack;
class QLineEdit;
class QAction;
namespace Seamly3D
{
class Viewport;
class ContourView;
class Window : public QMainWindow
{
public:
    Window();
    bool openProject(const QString &path);
    void showDemo();
    void showDressFormDemo();
    void advance(int iterations);
    bool render(const QString &path);
    bool rendererReady() const;
    QString rendererError() const;
    QString graphicsBackend() const;
    bool saveToPath(const QString &path);
    bool importFromPath(const QString &path);
protected:
    void closeEvent(QCloseEvent *) override;
private:
    Project m_project;
    Assembly m_assembly;
    QString m_path;
    Viewport *m_view;
    ContourView *m_contours[2];
    QComboBox *m_pieces[2];
    QDoubleSpinBox *m_start[2],*m_end[2];
    QCheckBox *m_reverse;
    QLabel *m_lengths,*m_metrics;
    QListWidget *m_seams;
    QComboBox *m_posePiece;
    QDoubleSpinBox *m_pose[6];
    QPushButton *m_run,*m_add;
    QTimer *m_timer;
    QTabWidget *m_inspector;
    QLabel *m_stateSummary,*m_backend,*m_pieceHint,*m_seamsHeading;
    QPushButton *m_remove;
    QGroupBox *m_formGroup;
    QCheckBox *m_formContact,*m_formStand;
    QDoubleSpinBox *m_formMeasures[7];
    QPushButton *m_arrange,*m_place;
    QComboBox *m_placement;
    QUndoStack *m_history;
    QListWidget *m_pieceList;
    QCheckBox *m_transparent;
    QPushButton *m_isolate,*m_showAll;
    QLabel *m_pieceListHeading;
    QLineEdit *m_seamName;
    QCheckBox *m_seamEnabled;
    QPushButton *m_applySeam,*m_revertSeam,*m_newSeam;
    QLabel *m_seamFeedback;
    QAction *m_undoAction,*m_redoAction;
    QString m_editSeamId;
    QDoubleSpinBox *m_meshSize,*m_clothValues[5];
    QComboBox *m_solverMode;
    QCheckBox *m_pinTop;
    QLabel *m_meshInfo,*m_solverInfo;
    void updateSimulationSettings();
    int editedSeamRow() const;
    Seam draftSeam() const;
    bool seamDraftDirty() const;
    void loadSeamEditor(const QString &id);
    void updateSeamDraft();
    bool applySeamDraft();
    void refreshSeamPreview();
    int m_iterations=0;
    bool m_updating=false;
    Piece m_dragStart;
    bool m_dragModified=false;
    Project m_dragBefore;
    bool m_unsavedOrigin=false;
    bool commitEdit(const Project &before,const QString &label);
    void restoreHistory(const Project &project);
    void updateDirty();
    void syncInspection();
    void applyTheme();
    bool acceptProject(const Project &project, const QString &path);
    void sync();
    void updateArcs();
    void updateDressForm();
    void arrangeOnForm(bool all);
    void updateMetrics();
    bool rebuild();
    void stop();
    bool saveProject(bool saveAs);
    bool maybeSave();
    void error(const QString &message);
    void importGeometry();
    void exportObj();
};
}
#endif
