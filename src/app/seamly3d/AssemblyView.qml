// SPDX-License-Identifier: GPL-3.0-or-later
import QtQuick
import QtQuick3D
import Seamly3D 1.0

Item {
    id: root
    View3D {
        id: scene
        anchors.fill: parent
        camera: camera
        environment: SceneEnvironment {
            backgroundMode: SceneEnvironment.Color
            clearColor: "#181f28"
            antialiasingMode: SceneEnvironment.MSAA
            antialiasingQuality: SceneEnvironment.High
        }
        Node {
            position: assemblyView.sceneCenter
            eulerRotation: Qt.vector3d(assemblyView.pitch, assemblyView.yaw, 0)
            OrthographicCamera {
                id: camera
                x: assemblyView.cameraPan.x
                y: assemblyView.cameraPan.y
                z: assemblyView.sceneRadius * 4
                horizontalMagnification: assemblyView.magnification
                verticalMagnification: assemblyView.magnification
                clipNear: Math.max(0.01, assemblyView.sceneRadius * 0.001)
                clipFar: assemblyView.sceneRadius * 12
            }
        }
        DirectionalLight {
            eulerRotation: Qt.vector3d(-35, -35, 0)
            brightness: 1.2
            ambientColor: "#606875"
            castsShadow: true
            shadowFactor: 45
            shadowMapQuality: Light.ShadowMapQualityHigh
            shadowBias: 0.1
        }
        DirectionalLight {
            eulerRotation: Qt.vector3d(25, 140, 0)
            brightness: 0.55
        }
        Model {
            source: "#Rectangle"
            position: Qt.vector3d(assemblyView.sceneCenter.x,
                                  assemblyView.floorHeight,
                                  assemblyView.sceneCenter.z)
            eulerRotation.x: -90
            scale: Qt.vector3d(assemblyView.sceneRadius * 0.05,
                               assemblyView.sceneRadius * 0.05, 1)
            castsShadows: false
            materials: PrincipledMaterial {
                baseColor: "#283442"
                roughness: 1
                cullMode: Material.NoCulling
            }
        }
        Node {
            visible: assemblyView.dressFormEnabled
            Model {
                property string pieceId: ""
                objectName: "dressFormBody"
                receivesShadows: false
                geometry: assemblyView.dressFormGeometry
                pickable: true
                materials: PrincipledMaterial {
                    baseColor: "#d7c9ad"
                    roughness: 0.93
                    specularAmount: 0.12
                }
            }
            Model {
                source: "#Cylinder"
                y: assemblyView.dressFormTop - 1
                scale: Qt.vector3d(assemblyView.dressFormNeck.x * 2.03 / 100,
                                   0.12, assemblyView.dressFormNeck.z * 2.03 / 100)
                materials: PrincipledMaterial { baseColor: "#d0b475"; metalness: 0.35; roughness: 0.3 }
            }
            Model {
                visible: assemblyView.dressFormStand
                source: "#Cylinder"
                y: (assemblyView.dressFormBottom + assemblyView.dressFormFloor) / 2
                scale: Qt.vector3d(0.14, (assemblyView.dressFormBottom - assemblyView.dressFormFloor) / 100, 0.14)
                materials: PrincipledMaterial { baseColor: "#b6b5ab"; metalness: 0.4; roughness: 0.4 }
            }
            Model {
                visible: assemblyView.dressFormStand
                source: "#Cylinder"
                y: assemblyView.dressFormFloor + 3
                scale: Qt.vector3d(3.1, 0.10, 3.1)
                materials: PrincipledMaterial { baseColor: "#46494c"; metalness: 0.7; roughness: 0.4 }
            }
        }
        Repeater3D {
            model: assemblyView.pieces
            delegate: Node {
                required property var modelData
                id: pieceNode
                visible: modelData.shown
                Model {
                    property string pieceId: pieceNode.modelData.pieceId
                    objectName: "pieceModel_" + pieceId
                    geometry: pieceNode.modelData.surface
                    pickable: true
                    materials: PrincipledMaterial {
                        baseColor: pieceNode.modelData.color
                        roughness: 0.82
                        metalness: 0
                        specularAmount: 0.25
                        cullMode: Material.NoCulling
                        opacity: pieceNode.modelData.ghosted ? 0.28 : 1
                        alphaMode: pieceNode.modelData.ghosted ? PrincipledMaterial.Blend : PrincipledMaterial.Opaque
                        emissiveFactor: assemblyView.selectedId === pieceNode.modelData.pieceId
                                        ? Qt.vector3d(0.16, 0.13, 0.04) : Qt.vector3d(0, 0, 0)
                    }
                }
                Model {
                    geometry: pieceNode.modelData.wire
                    visible: assemblyView.wireframe && !pieceNode.modelData.ghosted
                    depthBias: -0.5
                    castsShadows: false
                    materials: DefaultMaterial {
                        diffuseColor: "#243442"
                        lighting: DefaultMaterial.NoLighting
                    }
                }
                Model {
                    geometry: pieceNode.modelData.sewing
                    visible: pieceNode.modelData.hasSewing && !pieceNode.modelData.ghosted
                    castsShadows: false
                    materials: DefaultMaterial {
                        diffuseColor: "#ffd269"
                        lighting: DefaultMaterial.NoLighting
                        cullMode: Material.NoCulling
                    }
                }
            }
        }
    }
    Canvas {
        id: seamGuides
        anchors.fill: parent
        visible: assemblyView.seamPreviewActive
        onVisibleChanged: requestPaint()
        onWidthChanged: requestPaint()
        onHeightChanged: requestPaint()
        Connections {
            target: assemblyView
            function onSeamPreviewChanged() { seamGuides.requestPaint() }
            function onCameraChanged() { seamGuides.requestPaint() }
            function onInspectionChanged() { seamGuides.requestPaint() }
        }
        onPaint: {
            const ctx=getContext("2d"); ctx.reset()
            const guides=assemblyView.seamGuides
            // Inspection overlays deliberately reveal both ranges through geometry.
            for (let pass=0;pass<2;++pass) for (const guide of guides) {
                ctx.beginPath()
                for (let i=0;i<guide.points.length;++i) {
                    const p=scene.mapFrom3DScene(guide.points[i])
                    if (i===0) ctx.moveTo(p.x,p.y); else ctx.lineTo(p.x,p.y)
                }
                ctx.lineJoin="round"; ctx.lineCap="round"
                ctx.setLineDash(pass===1 && guide.side===1 ? [5,5] : [])
                ctx.lineWidth=pass===0 ? 7 : 3
                ctx.strokeStyle=pass===0 ? "#16221e" : (guide.side===0 ? "#70c9ed" : "#f3a48d")
                ctx.stroke()
            }
        }
    }
    Column {
        visible: assemblyView.seamPreviewActive
        anchors { right: parent.right; bottom: parent.bottom; margins: 18 }
        spacing: 4
        Text { text: assemblyView.seamPreviewName || qsTr("New seam"); color: "#bdccdb"; font.pixelSize: 12; width: 190; elide: Text.ElideRight }
        Text { text: qsTr("Side A — solid · Side B — dashed"); color: "#bdccdb"; font.pixelSize: 11 }
        Text { text: qsTr("Guides show through geometry"); color: "#bdccdb"; font.pixelSize: 11 }
    }
    Canvas {
        id: handles
        objectName: "placementHandles"
        anchors.fill: parent
        visible: assemblyView.toolMode > 0 && assemblyView.selectedVisible
        property var geometry: []
        property int hoverAxis: -1
        readonly property var axisColors: ["#ef9586", "#b4d69a", "#92bcf4"]
        function projected(v) { return scene.mapFrom3DScene(v) }
        function pointAlong(origin, direction, amount) {
            return Qt.vector3d(origin.x + direction.x * amount, origin.y + direction.y * amount, origin.z + direction.z * amount)
        }
        function cross(a,b) { return Qt.vector3d(a.y*b.z-a.z*b.y,a.z*b.x-a.x*b.z,a.x*b.y-a.y*b.x) }
        function rebuildGeometry() {
            if (!visible) { geometry = []; return }
            const origin = assemblyView.placementOrigin
            const center = projected(origin)
            const radius = (assemblyView.toolMode === 2 ? 68 : 96) / assemblyView.magnification
            let items = []
            for (let axis = 0; axis < 3; ++axis) {
                const direction = assemblyView.handleAxis(axis)
                if (assemblyView.toolMode === 1) {
                    const end = projected(pointAlong(origin, direction, radius))
                    const dx = end.x-center.x, dy = end.y-center.y
                    if (dx*dx+dy*dy > 324)
                        items.push({axis:axis, center:center, end:end, radius:radius})
                } else {
                    const reference = Math.abs(direction.y) < 0.9 ? Qt.vector3d(0,1,0) : Qt.vector3d(1,0,0)
                    const raw = cross(direction, reference)
                    const length = Math.sqrt(raw.x*raw.x+raw.y*raw.y+raw.z*raw.z)
                    const u = Qt.vector3d(raw.x/length,raw.y/length,raw.z/length)
                    const v = cross(direction,u)
                    const a = projected(pointAlong(origin,u,radius))
                    const b = projected(pointAlong(origin,v,radius))
                    const ux=a.x-center.x, uy=a.y-center.y, vx=b.x-center.x, vy=b.y-center.y
                    const determinant=ux*vy-uy*vx
                    // Edge-on rings have no reliable mouse angle. Orbit or use numeric controls.
                    if (Math.abs(determinant) > 500)
                        items.push({axis:axis,center:center,ux:ux,uy:uy,vx:vx,vy:vy,det:determinant})
                }
            }
            geometry = items
            requestPaint()
        }
        function angle(item,x,y) {
            const dx=x-item.center.x,dy=y-item.center.y
            return Math.atan2((item.ux*dy-item.uy*dx)/item.det,(dx*item.vy-dy*item.vx)/item.det)
        }
        function hit(x,y) {
            let nearest=null, distance=9
            for (const item of geometry) {
                let d=10000
                if (assemblyView.toolMode === 1) {
                    const dx=item.end.x-item.center.x,dy=item.end.y-item.center.y
                    const t=Math.max(0.2,Math.min(1,((x-item.center.x)*dx+(y-item.center.y)*dy)/(dx*dx+dy*dy)))
                    d=Math.hypot(x-item.center.x-t*dx,y-item.center.y-t*dy)
                } else {
                    const t=angle(item,x,y)
                    d=Math.hypot(x-item.center.x-item.ux*Math.cos(t)-item.vx*Math.sin(t),
                                 y-item.center.y-item.uy*Math.cos(t)-item.vy*Math.sin(t))
                }
                if (d<distance) { nearest=item; distance=d }
            }
            return nearest
        }
        onVisibleChanged: rebuildGeometry()
        onWidthChanged: rebuildGeometry()
        onHeightChanged: rebuildGeometry()
        onHoverAxisChanged: requestPaint()
        Connections {
            target: assemblyView
            function onCameraChanged() { handles.rebuildGeometry() }
            function onSelectionChanged() { handles.rebuildGeometry() }
            function onToolModeChanged() { handles.rebuildGeometry() }
        }
        onPaint: {
            const ctx=getContext("2d"); ctx.reset()
            for (const item of geometry) {
                const active=hoverAxis===item.axis
                ctx.beginPath()
                if (assemblyView.toolMode===1) {
                    ctx.moveTo(item.center.x,item.center.y); ctx.lineTo(item.end.x,item.end.y)
                } else {
                    for (let i=0;i<=96;++i) {
                        const t=i*Math.PI*2/96
                        const x=item.center.x+item.ux*Math.cos(t)+item.vx*Math.sin(t)
                        const y=item.center.y+item.uy*Math.cos(t)+item.vy*Math.sin(t)
                        if (i===0) ctx.moveTo(x,y); else ctx.lineTo(x,y)
                    }
                }
                ctx.strokeStyle="#16221e"; ctx.lineWidth=active?7:6; ctx.stroke()
                ctx.strokeStyle=axisColors[item.axis]; ctx.lineWidth=active?3.5:2.5; ctx.stroke()
                const x=assemblyView.toolMode===1?item.end.x:item.center.x+item.ux
                const y=assemblyView.toolMode===1?item.end.y:item.center.y+item.uy
                ctx.beginPath(); ctx.arc(x,y,active?7:5,0,Math.PI*2)
                ctx.fillStyle=axisColors[item.axis]; ctx.fill()
                // Labels must stay legible over both linen and bright cloth.
                ctx.fillStyle="#16221e"; ctx.fillRect(x+8,y-25,20,20)
                ctx.font="bold 12px sans-serif"; ctx.textAlign="center"; ctx.textBaseline="middle"
                ctx.fillStyle=axisColors[item.axis]; ctx.fillText(["X","Y","Z"][item.axis],x+18,y-15)
            }
        }
    }
    MouseArea {
        id: navigation
        anchors.fill: parent
        acceptedButtons: Qt.LeftButton | Qt.RightButton | Qt.MiddleButton
        hoverEnabled: true
        focus: true
        property point previous
        property point origin
        property bool moved: false
        property var handle: null
        property real lastAngle: 0
        property real totalAngle: 0
        cursorShape: handle || handles.hoverAxis>=0 ? Qt.SizeAllCursor : Qt.ArrowCursor
        function cancelDrag() { assemblyView.endTransform(true); handle=null; handles.hoverAxis=-1; handles.requestPaint() }
        Keys.onEscapePressed: cancelDrag()
        onCanceled: cancelDrag()
        onPressed: function(mouse) {
            forceActiveFocus()
            previous=Qt.point(mouse.x,mouse.y); origin=previous; moved=false
            handle=mouse.button===Qt.LeftButton && handles.visible ? handles.hit(mouse.x,mouse.y) : null
            if (handle && assemblyView.beginTransform()) {
                lastAngle=handles.angle(handle,mouse.x,mouse.y); totalAngle=0
            } else handle=null
        }
        onPositionChanged: function(mouse) {
            if (!pressed) {
                const hit=handles.visible?handles.hit(mouse.x,mouse.y):null
                handles.hoverAxis=hit?hit.axis:-1; handles.requestPaint(); return
            }
            const dx=mouse.x-previous.x,dy=mouse.y-previous.y
            if (Math.abs(mouse.x-origin.x)+Math.abs(mouse.y-origin.y)>4) moved=true
            if (handle) {
                if (assemblyView.toolMode===1) {
                    const ax=handle.end.x-handle.center.x,ay=handle.end.y-handle.center.y
                    assemblyView.transformBy(handle.axis,((mouse.x-origin.x)*ax+(mouse.y-origin.y)*ay)/(ax*ax+ay*ay)*handle.radius)
                } else {
                    const current=handles.angle(handle,mouse.x,mouse.y)
                    let delta=current-lastAngle
                    if (delta>Math.PI) delta-=Math.PI*2
                    if (delta< -Math.PI) delta+=Math.PI*2
                    totalAngle+=delta; lastAngle=current
                    assemblyView.transformBy(handle.axis,totalAngle*180/Math.PI)
                }
            } else if (mouse.buttons & Qt.LeftButton) assemblyView.orbit(dx,dy)
            else assemblyView.pan(dx,dy)
            previous=Qt.point(mouse.x,mouse.y)
        }
        onReleased: function(mouse) {
            if (handle) { assemblyView.endTransform(false); handle=null; return }
            if (!moved && mouse.button===Qt.LeftButton) {
                const result=scene.pick(mouse.x,mouse.y)
                assemblyView.selectPiece(result.objectHit?result.objectHit.pieceId:"")
            }
        }
        onDoubleClicked: assemblyView.fit()
        onWheel: function(wheel) { if (!handle) assemblyView.zoom(wheel.angleDelta.y); wheel.accepted=true }
    }
    Text {
        anchors { left: parent.left; top: parent.top; margins: 18 }
        color: "#bdccdb"
        width: Math.max(0, parent.width - 90)
        elide: Text.ElideRight
        text: assemblyView.selectedName || (assemblyView.pieces.length ? qsTr("Assembly")
                    : (assemblyView.dressFormEnabled ? qsTr("Dress form") : qsTr("Open a project or an example to begin.")))
        font.pixelSize: 12
    }
    Text {
        anchors { right: parent.right; top: parent.top; margins: 18 }
        color: "#91b5c6"
        text: "mm"
        font.pixelSize: 12
    }
    Text {
        anchors { left: parent.left; bottom: parent.bottom; margins: 18 }
        color: "#bdccdb"
        text: assemblyView.toolMode === 0 ? qsTr("Drag to orbit   ·   Scroll to zoom") :
              (assemblyView.selectedId === "" ? qsTr("Select a piece to edit its placement") :
              (assemblyView.toolMode === 1 ? qsTr("Drag an axis to move   ·   Esc cancels") : qsTr("Drag a ring to rotate   ·   Esc cancels")))
        font.pixelSize: 12
    }
}
