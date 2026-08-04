## Explicit source list for vtools (converted from vtools.pri and included pri files)
set(vtools_sources
		dialogs/support/edit_formula_dialog.cpp
		dialogs/support/editlabeltemplate_dialog.cpp
		dialogs/tools/addtogroup_dialog.cpp
		dialogs/tools/anchorpoint_dialog.cpp
		dialogs/tools/dialogalongline.cpp
		dialogs/tools/dialogarc.cpp
		dialogs/tools/dialogarcwithlength.cpp
		dialogs/tools/dialogbisector.cpp
		dialogs/tools/dialogcurveintersectaxis.cpp
		dialogs/tools/dialogcutarc.cpp
		dialogs/tools/dialogcutspline.cpp
		dialogs/tools/dialogcutsplinepath.cpp
		dialogs/tools/dialogendline.cpp
		dialogs/tools/dialogheight.cpp
		dialogs/tools/dialogline.cpp
		dialogs/tools/dialoglineintersect.cpp
		dialogs/tools/dialoglineintersectaxis.cpp
		dialogs/tools/dialognormal.cpp
		dialogs/tools/dialogpointfromarcandtangent.cpp
		dialogs/tools/dialogpointofcontact.cpp
		dialogs/tools/dialogpointofintersectionarcs.cpp
		dialogs/tools/dialogshoulderpoint.cpp
		dialogs/tools/dialogsinglepoint.cpp
		dialogs/tools/dialogspline.cpp
		dialogs/tools/dialogsplinepath.cpp
		dialogs/tools/dialogtool.cpp
		dialogs/tools/dialogtriangle.cpp
		dialogs/support/dialogundo.cpp
		dialogs/tools/dialogtruedarts.cpp
		dialogs/tools/dialogpointofintersectioncurves.cpp
		dialogs/tools/dialogcubicbezier.cpp
		dialogs/tools/dialogcubicbezierpath.cpp
		dialogs/tools/dialogrotation.cpp
		dialogs/tools/dialogmirrorbyline.cpp
		dialogs/tools/dialogmirrorbyaxis.cpp
		dialogs/tools/dialogmove.cpp
		dialogs/tools/dialogellipticalarc.cpp
		dialogs/tools/editgroup_dialog.cpp
		dialogs/tools/insert_nodes_dialog.cpp
		dialogs/tools/intersect_circles_dialog.cpp
		dialogs/tools/intersect_circletangent_dialog.cpp
		dialogs/tools/piece/internal_path_dialog.cpp
		dialogs/tools/piece/pattern_piece_dialog.cpp
		dialogs/tools/point_intersectxy_dialog.cpp
		dialogs/tools/union_dialog.cpp

		tools/drawTools/operation/mirror/vabstractmirror.cpp
		tools/drawTools/operation/mirror/vtoolmirrorbyaxis.cpp
		tools/drawTools/operation/mirror/vtoolmirrorbyline.cpp
		tools/drawTools/toolpoint/toolsinglepoint/point_intersectxy_tool.cpp
		tools/drawTools/toolpoint/toolsinglepoint/toollinepoint/doubleline_point_tool.cpp
		tools/drawTools/toolpoint/toolsinglepoint/intersect_circles_tool.cpp
		tools/drawTools/toolpoint/toolsinglepoint/intersect_circletangent_tool.cpp
		tools/nodeDetails/anchorpoint_tool.cpp
		tools/nodeDetails/internal_path_tool.cpp
		tools/pattern_piece_tool.cpp
		tools/union_tool.cpp
		tools/vdatatool.cpp
		tools/vabstracttool.cpp
		tools/drawTools/toolpoint/toolsinglepoint/vtooltriangle.cpp
		tools/drawTools/toolcurve/vtoolsplinepath.cpp
		tools/drawTools/toolcurve/vtoolspline.cpp
		tools/drawTools/toolpoint/toolsinglepoint/vtoolbasepoint.cpp
		tools/drawTools/toolpoint/toolsinglepoint/toollinepoint/vtoolshoulderpoint.cpp
		tools/drawTools/toolpoint/toolsinglepoint/vtoolpointofcontact.cpp
		tools/drawTools/toolpoint/toolsinglepoint/vtoolsinglepoint.cpp
		tools/drawTools/toolpoint/toolsinglepoint/toollinepoint/vtoolnormal.cpp
		tools/drawTools/toolpoint/toolsinglepoint/toollinepoint/vtoollinepoint.cpp
		tools/drawTools/toolpoint/toolsinglepoint/vtoollineintersect.cpp
		tools/drawTools/vtoolline.cpp
		tools/drawTools/toolpoint/toolsinglepoint/toollinepoint/vtoolheight.cpp
		tools/drawTools/toolpoint/toolsinglepoint/toollinepoint/vtoolendline.cpp
		tools/drawTools/toolpoint/toolsinglepoint/toollinepoint/vtoolbisector.cpp
		tools/drawTools/toolcurve/vtoolarc.cpp
		tools/drawTools/toolpoint/toolsinglepoint/toollinepoint/vtoolalongline.cpp
		tools/drawTools/vdrawtool.cpp
		tools/nodeDetails/vnodesplinepath.cpp
		tools/nodeDetails/vnodespline.cpp
		tools/nodeDetails/vnodepoint.cpp
		tools/nodeDetails/vnodearc.cpp
		tools/nodeDetails/vabstractnode.cpp
		tools/drawTools/toolpoint/toolsinglepoint/toolcut/vtoolcutspline.cpp
		tools/drawTools/toolpoint/toolsinglepoint/toolcut/vtoolcutsplinepath.cpp
		tools/drawTools/toolpoint/toolsinglepoint/toolcut/vtoolcutarc.cpp
		tools/drawTools/toolcurve/vabstractspline.cpp
		tools/drawTools/toolpoint/toolsinglepoint/toolcut/vtoolcut.cpp
		tools/drawTools/toolpoint/toolsinglepoint/toollinepoint/vtoollineintersectaxis.cpp
		tools/drawTools/toolpoint/toolsinglepoint/toollinepoint/vtoolcurveintersectaxis.cpp
		tools/drawTools/toolpoint/toolsinglepoint/vtoolpointofintersectionarcs.cpp
		tools/drawTools/toolpoint/toolsinglepoint/vtoolpointfromarcandtangent.cpp
		tools/drawTools/toolcurve/vtoolarcwithlength.cpp
		tools/drawTools/toolpoint/vabstractpoint.cpp
		tools/drawTools/toolpoint/tooldoublepoint/vtooldoublepoint.cpp
		tools/drawTools/toolpoint/tooldoublepoint/vtooltruedarts.cpp
		tools/drawTools/toolpoint/toolsinglepoint/vtoolpointofintersectioncurves.cpp
		tools/drawTools/toolcurve/vtoolcubicbezier.cpp
		tools/drawTools/toolcurve/vtoolcubicbezierpath.cpp
		tools/drawTools/operation/vtoolrotation.cpp
		tools/drawTools/operation/vabstractoperation.cpp
		tools/drawTools/operation/vtoolmove.cpp
		tools/drawTools/toolcurve/vtoolellipticalarc.cpp
		tools/nodeDetails/vnodeellipticalarc.cpp
		tools/vinteractivetool.cpp

		visualization/line/anchorpoint_visual.cpp
		visualization/line/intersect_circles_visual.cpp
		visualization/line/intersect_circletangent_visual.cpp
		visualization/line/operation/vistoolmirrorbyaxis.cpp
		visualization/line/operation/vistoolmirrorbyline.cpp
		visualization/path/internal_path_visual.cpp
		visualization/path/pattern_piece_visual.cpp
		visualization/path/pieceanchorpoint_visual.cpp
		visualization/line/point_intersectxy_visual.cpp
		visualization/visualization.cpp
		visualization/line/visline.cpp
		visualization/line/vistoolline.cpp
		visualization/line/vistoolendline.cpp
		visualization/line/vistoolalongline.cpp
		visualization/line/vistoolbisector.cpp
		visualization/line/vistoolshoulderpoint.cpp
		visualization/line/vistoolnormal.cpp
		visualization/line/vistoolheight.cpp
		visualization/line/vistooltriangle.cpp
		visualization/line/vistoolpointofcontact.cpp
		visualization/line/vistoollineintersect.cpp
		visualization/line/vistoollineintersectaxis.cpp
		visualization/line/vistooltruedarts.cpp
		visualization/line/vistoolcurveintersectaxis.cpp
		visualization/line/vistoolpointofintersectionarcs.cpp
		visualization/line/vistoolpointfromarcandtangent.cpp
		visualization/line/operation/vistoolrotation.cpp
		visualization/path/vispath.cpp
		visualization/path/vistoolarc.cpp
		visualization/path/vistoolcutarc.cpp
		visualization/path/vistoolspline.cpp
		visualization/path/vistoolcutspline.cpp
		visualization/path/vistoolsplinepath.cpp
		visualization/path/vistoolcutsplinepath.cpp
		visualization/path/vistoolarcwithlength.cpp
		visualization/path/vistoolpointofintersectioncurves.cpp
		visualization/path/vistoolcubicbezier.cpp
		visualization/path/vistoolcubicbezierpath.cpp
		visualization/line/operation/visoperation.cpp
		visualization/line/operation/vistoolmove.cpp
		visualization/path/vistoolellipticalarc.cpp

		undocommands/add_draftblock.cpp
		undocommands/add_groupitem.cpp
		undocommands/addtocalc.cpp
		undocommands/delete_draftblock.cpp
		undocommands/label/moveoperationlabel.cpp
		undocommands/label/showoperationpointname.cpp
		undocommands/move_groupitem.cpp
		undocommands/movespoint.cpp
		undocommands/movespline.cpp
		undocommands/movesplinepath.cpp
		undocommands/remove_groupitem.cpp
		undocommands/rename_draftblock.cpp
		undocommands/savetooloptions.cpp
		undocommands/deltool.cpp
		undocommands/adddetnode.cpp
		undocommands/set_piece_color.cpp
		undocommands/toggle_piecelock.cpp
		undocommands/vundocommand.cpp
		undocommands/label/movelabel.cpp
		undocommands/label/movedoublelabel.cpp
		undocommands/label/showpointname.cpp
		undocommands/label/showdoublepointname.cpp
		undocommands/addgroup.cpp
		undocommands/delgroup.cpp
		undocommands/label/moveabstractlabel.cpp
		undocommands/addpiece.cpp
		undocommands/addimage.cpp
		undocommands/deletepiece.cpp
		undocommands/movepiece.cpp
		undocommands/savepieceoptions.cpp
		undocommands/togglepieceinlayout.cpp
		undocommands/savepiecepathoptions.cpp
)

# Include stable.cpp only on MSVC (matches vtools.pri behavior)
if(MSVC)
	list(APPEND vtools_sources stable.cpp)
endif()

## UI forms (AUTOUIC will process these)
set(vtools_forms
		dialogs/support/edit_formula_dialog.ui
		dialogs/support/editlabeltemplate_dialog.ui
		dialogs/tools/addtogroup_dialog.ui
		dialogs/tools/anchorpoint_dialog.ui
		dialogs/tools/dialogalongline.ui
		dialogs/tools/dialogarc.ui
		dialogs/tools/dialogarcwithlength.ui
		dialogs/tools/dialogbisector.ui
		dialogs/tools/dialogcurveintersectaxis.ui
		dialogs/tools/dialogcutarc.ui
		dialogs/tools/dialogcutspline.ui
		dialogs/tools/dialogcutsplinepath.ui
		dialogs/tools/dialogendline.ui
		dialogs/tools/dialogheight.ui
		dialogs/tools/dialogline.ui
		dialogs/tools/dialoglineintersect.ui
		dialogs/tools/dialoglineintersectaxis.ui
		dialogs/tools/dialognormal.ui
		dialogs/tools/dialogpointfromarcandtangent.ui
		dialogs/tools/dialogpointofcontact.ui
		dialogs/tools/dialogpointofintersectionarcs.ui
		dialogs/tools/dialogshoulderpoint.ui
		dialogs/tools/dialogsinglepoint.ui
		dialogs/tools/dialogspline.ui
		dialogs/tools/dialogsplinepath.ui
		dialogs/tools/dialogtriangle.ui
		dialogs/support/dialogundo.ui
		dialogs/tools/dialogtruedarts.ui
		dialogs/tools/dialogpointofintersectioncurves.ui
		dialogs/tools/dialogcubicbezier.ui
		dialogs/tools/dialogcubicbezierpath.ui
		dialogs/tools/dialogrotation.ui
		dialogs/tools/dialogmirrorbyline.ui
		dialogs/tools/dialogmirrorbyaxis.ui
		dialogs/tools/dialogmove.ui
		dialogs/tools/dialogellipticalarc.ui
		dialogs/tools/editgroup_dialog.ui
		dialogs/tools/insert_nodes_dialog.ui
		dialogs/tools/intersect_circles_dialog.ui
		dialogs/tools/intersect_circletangent_dialog.ui
		dialogs/tools/piece/internal_path_dialog.ui
		dialogs/tools/piece/pattern_piece_dialog.ui
		dialogs/tools/point_intersectxy_dialog.ui
		dialogs/tools/union_dialog.ui
)

list(APPEND vtools_sources ${vtools_forms})

add_library(vtools STATIC ${vtools_sources})
target_include_directories(vtools PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/vtools_autogen/include>
)
target_link_libraries(vtools PUBLIC Qt6::Widgets Qt6::Xml Qt6::PrintSupport Qt6::Multimedia Qt6::Svg Qt6::Network)
target_link_libraries(vtools PUBLIC vpatterndb libs_common_include)
target_compile_features(vtools PUBLIC cxx_std_14)

# Exclude legacy duplicate dialoggroup files from AUTOGEN/AUTOUIC to avoid
# duplicate `EditGroupDialog` metaobject and UI header conflicts. We do not
# modify source files; instead tell CMake to skip automoc/autouic for them.
## Ensure legacy `dialoggroup` sources are excluded from automoc/autouic
# Use source-relative paths so CMake matches the target's source list correctly.
set_source_files_properties(
    dialogs/tools/dialoggroup.h
    dialogs/tools/dialoggroup.cpp
    PROPERTIES SKIP_AUTOGEN ON)

set_source_files_properties(
    dialogs/tools/dialoggroup.ui
    PROPERTIES SKIP_AUTOUIC ON)
