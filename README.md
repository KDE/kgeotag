# KGeoTag

**Warning**: This is pre-0.1 beta code. It works ("on my machine" ;-), but until version 0.1 is tagged, use carefully. Have backups.

KGeoTag is a Free/Libre Open Source photo geotagging program. It's written in C++/[Qt](https://www.qt.io/) and uses the [KDE Frameworks](https://api.kde.org/frameworks/). It's published under the terms of the [GNU General Public License (GPL)](https://www.gnu.org/licenses/#GPL).

The program shows a map on which geodata stored in the [GPX](https://en.wikipedia.org/wiki/GPS_Exchange_Format) format can be displayed. Using this data, images can be matched with geographic coordinates automatically, either by finding (more or less) exact chronological matches, or by interpolating a likely position if no exact match can be found.\
Additionally, coordinates can be set or corrected manually by dragging and dropping images on the map directly. This also can be done without loading any geodata.

If the images' dates have a time shift due to the camera's clock being not exactly in sync with the GPS data (which is assumed to be correct), a deviation can be set and will be considered when searching for matches.

Finally, the assigned coordinates can be saved in the images' [Exif](https://en.wikipedia.org/wiki/Exif) header, making them persistent and also accessible for other applications.\
If a time drift has been identified and a deviation has been given, the images' dates and times also can be fixed whilst saving.
