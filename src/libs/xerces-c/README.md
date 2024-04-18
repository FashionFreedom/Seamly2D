xerces-c headers and static libs
================================

xerces is used in the ifc lib to perform XML schema validation, as that was removed from Qt6. If Qt regains the ability to perform XML schema validation,
this can be removed again and replaced with Qt functionality. Other Qt projects follow a similar approach to use xerces-c for XML schema validation.

The binaries and headers here are built with https://github.com/FashionFreedom/xerces-c/blob/main/.github/workflows/ci.yml

The binary releases from https://github.com/FashionFreedom/xerces-c are put under the arch specific folders, stripped from shared libraries and pkgconfig as
we link statically only the small required parts into Seamly2D and SeamlyME.
