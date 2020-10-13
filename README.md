# KGeoTag

**Warning**: This is pre-0.1 beta code. It works ("on my machine" ;-), but until version 0.1 is tagged, use carefully. Have backups.

KGeoTag is a standalone photo geotagging program for the [KDE](https://kde.org/) ecosystem, with the Unix philosophy in mind ("do one thing and do it well").

It shows a map on which geodata stored in [GPX](https://en.wikipedia.org/wiki/GPS_Exchange_Format) files can be displayed. Using this data, images can be matched with geographic coordinates automatically, either by finding (more or less) exact chronological matches or by interpolating a possible position if no exact match can be found. Additionally, coordinates can be set or corrected manually by dragging and dropping images on the map directly, which also can be done without additional geodata.

Finally, the assigned coordinates can be saven in the image's [Exif](https://en.wikipedia.org/wiki/Exif) header, making them persistent and accessible for other applications.

It uses the [Qt framework](https://www.qt.io/), [Marble](https://marble.kde.org/) for the map view and [libkexiv2](https://invent.kde.org/graphics/libkexiv2) for image metadata reading and writing.
