// Run from any directory; the banner is resolved relative to this source file:
// swift dist/macx/dmg/render-background.swift dist/macx/dmg/background.tiff
import AppKit

let assets = URL(fileURLWithPath: #filePath).deletingLastPathComponent()
let bannerURL = assets.appendingPathComponent("../../../share/img/Seamly2D_banner_1202x271.png").standardized
guard let banner = NSImage(contentsOf: bannerURL) else {
    fatalError("Cannot load Seamly banner at \(bannerURL.path)")
}
let size = NSSize(width: 640, height: 400)
let image = NSImage(size: size)
for scale in [1, 2] {
    let rep = NSBitmapImageRep(bitmapDataPlanes: nil, pixelsWide: 640 * scale,
        pixelsHigh: 400 * scale, bitsPerSample: 8, samplesPerPixel: 4,
        hasAlpha: true, isPlanar: false, colorSpaceName: .deviceRGB,
        bytesPerRow: 0, bitsPerPixel: 0)!
    rep.size = size
    NSGraphicsContext.saveGraphicsState()
    NSGraphicsContext.current = NSGraphicsContext(bitmapImageRep: rep)
    NSColor.white.setFill()
    NSRect(origin: .zero, size: size).fill()
    banner.draw(in: NSRect(x: 40, y: 234, width: 560, height: 560 * 271 / 1202))

    // A hand-drawn curl guides the app icon toward the Applications folder.
    let arrow = NSBezierPath()
    arrow.move(to: NSPoint(x: 266, y: 130))
    arrow.curve(to: NSPoint(x: 318, y: 151), controlPoint1: NSPoint(x: 280, y: 164), controlPoint2: NSPoint(x: 302, y: 170))
    arrow.curve(to: NSPoint(x: 312, y: 125), controlPoint1: NSPoint(x: 334, y: 132), controlPoint2: NSPoint(x: 326, y: 115))
    arrow.curve(to: NSPoint(x: 343, y: 149), controlPoint1: NSPoint(x: 292, y: 138), controlPoint2: NSPoint(x: 322, y: 160))
    arrow.curve(to: NSPoint(x: 378, y: 131), controlPoint1: NSPoint(x: 358, y: 144), controlPoint2: NSPoint(x: 366, y: 135))
    arrow.move(to: NSPoint(x: 366, y: 147))
    arrow.line(to: NSPoint(x: 378, y: 131))
    arrow.line(to: NSPoint(x: 358, y: 131))
    NSColor(srgbRed: 76 / 255, green: 54 / 255, blue: 127 / 255, alpha: 1).setStroke()
    arrow.lineWidth = 3.5
    arrow.lineCapStyle = .round
    arrow.lineJoinStyle = .round
    arrow.stroke()
    NSGraphicsContext.restoreGraphicsState()
    image.addRepresentation(rep)
}
let data = image.tiffRepresentation(using: .lzw, factor: 1)!
try data.write(to: URL(fileURLWithPath: CommandLine.arguments[1]))
