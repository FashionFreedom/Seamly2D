The build copies the prepared Finder layout for each volume to .DS_Store.
No Finder automation or Python packages are needed during packaging.

Template          Volume     App bundle
Seamly2D.DS_Store  Seamly2D   Seamly2D.app
SeamlyME.DS_Store  SeamlyME   seamlyme.app

Both templates reference /.background/background.tiff within their volume.
They contain no build-machine paths or file IDs. Keep the volume and bundle
names above when packaging, or update the corresponding template.

The templates set a 640 x 432 window, 96-point icons, and app/Applications
positions (170, 260) and (470, 260). To change these settings, edit the
binary templates with the ds_store Python package (used only for template
maintenance). The icvp record holds the background and icon settings;
bwsp holds the window settings; each item's Iloc record holds its position.

render-background.swift regenerates the background artwork independently.
