# ADD TO EACH PATH $$PWD VARIABLE!!!!!!
# This need for corect working file translations.pro

SOURCES += \
    $$PWD/calculator/button.cpp \
    $$PWD/calculator/calculator.cpp \
    $$PWD/color_combobox.cpp \
    $$PWD/export_format_combobox.cpp \
    $$PWD/fill_combobox.cpp \
    $$PWD/group_tablewidgetitem.cpp \
    $$PWD/linetype_combobox.cpp \
    $$PWD/lineweight_combobox.cpp \
    $$PWD/mouse_coordinates.cpp \
    $$PWD/page_format_combobox.cpp \
    $$PWD/pen_toolbar.cpp \
    $$PWD/nonscalingfill_pathitem.cpp \
    $$PWD/piece_tablewidgetitem.cpp \
    $$PWD/scene_rect.cpp \
    $$PWD/vmaingraphicsscene.cpp \
    $$PWD/vmaingraphicsview.cpp \
    $$PWD/vwidgetpopup.cpp \
    $$PWD/vcontrolpointspline.cpp \
    $$PWD/vgraphicssimpletextitem.cpp \
    $$PWD/vsimplepoint.cpp \
    $$PWD/vabstractsimple.cpp \
    $$PWD/vsimplecurve.cpp \
    $$PWD/vabstractmainwindow.cpp \
    $$PWD/vtextgraphicsitem.cpp \
    $$PWD/vgrainlineitem.cpp \
    $$PWD/vpieceitem.cpp \
    $$PWD/vcurvepathitem.cpp \
    $$PWD/global.cpp \
    $$PWD/vscenepoint.cpp \
    $$PWD/scalesceneitems.cpp \
    $$PWD/vlineedit.cpp

*msvc*:SOURCES += $$PWD/stable.cpp

HEADERS += \
    $$PWD/calculator/button.h \
    $$PWD/calculator/calculator.h \
    $$PWD/color_combobox.h \
    $$PWD/export_format_combobox.h \
    $$PWD/fill_combobox.h \
    $$PWD/group_tablewidgetitem.h \
    $$PWD/linetype_combobox.h \
    $$PWD/lineweight_combobox.h \
    $$PWD/mouse_coordinates.h \
    $$PWD/page_format_combobox.h \
    $$PWD/pen_toolbar.h \
    $$PWD/nonscalingfill_pathitem.h \
    $$PWD/piece_tablewidgetitem.h \
    $$PWD/scene_rect.h \
    $$PWD/stable.h \
    $$PWD/vmaingraphicsscene.h \
    $$PWD/vmaingraphicsview.h \
    $$PWD/vwidgetpopup.h \
    $$PWD/vcontrolpointspline.h \
    $$PWD/vgraphicssimpletextitem.h \
    $$PWD/vsimplepoint.h \
    $$PWD/vabstractsimple.h \
    $$PWD/vsimplecurve.h \
    $$PWD/vabstractmainwindow.h \
    $$PWD/vtextgraphicsitem.h \
    $$PWD/vgrainlineitem.h \
    $$PWD/vpieceitem.h \
    $$PWD/vcurvepathitem.h \
    $$PWD/global.h \
    $$PWD/vscenepoint.h \
    $$PWD/scalesceneitems.h \
    $$PWD/vlineedit.h

FORMS += \
    $$PWD/mouse_coordinates.ui
