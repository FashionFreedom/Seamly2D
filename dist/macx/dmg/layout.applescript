-- Finder stores these settings in the image's .DS_Store.
on run arguments
    set mountPath to item 1 of arguments
    set appName to item 2 of arguments
    set imageFolder to POSIX file mountPath as alias
    tell application "Finder"
        open imageFolder
        set installerWindow to container window of imageFolder
        tell installerWindow
            set current view to icon view
            set toolbar visible to false
            set statusbar visible to false
            set pathbar visible to false
            set bounds to {200, 160, 840, 592}
        end tell
        set viewOptions to icon view options of installerWindow
        tell viewOptions
            set arrangement to not arranged
            set icon size to 96
            set text size to 13
            set label position to bottom
        end tell
        set background picture of viewOptions to file ".background:background.tiff" of imageFolder
        set position of item appName of imageFolder to {170, 240}
        set position of item "Applications" of imageFolder to {470, 240}
        close installerWindow
        open imageFolder
        update imageFolder without registering applications
        delay 2
        close container window of imageFolder
    end tell
end run
