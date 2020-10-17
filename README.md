# KGeoTag

**Warning**: This is pre-0.1 beta code. It works ("on my machine" ;-), but until version 0.1 is tagged, use carefully. Have backups.

## What is it?

KGeoTag is a Free/Libre Open Source photo geotagging program. It's written in C++/[Qt](https://www.qt.io/) and uses the [KDE Frameworks](https://api.kde.org/frameworks/). It's published under the terms of the [GNU General Public License (GPL)](https://www.gnu.org/licenses/#GPL).

## What is "geotagging"?

JPEG images contain metadata (e. g. the creation date, camera information etc.). Those are stored in the so-called [Exif header](https://en.wikipedia.org/wiki/Exif). This data can also represent geographic coordinates so that it's replicable where the images were taken.

## How does it work?

Most cameras don't have GPS receivers, so, most can't save coordinates when taking images. A common approach is to e. g. carry a small GPS logging device along, which records a track all the time. Later on, the images' dates can be compared to the GPS log's points' dates to figure out where an image was taken.\
It's also possible to manually select coordinates and add them to the images, if one knows for sure where the photo was taken.

### Assigning images

KGeoTag shows a map on which geodata stored in the [GPX](https://en.wikipedia.org/wiki/GPS_Exchange_Format) format can be displayed. Using this data, images can be matched with their respective geographic coordinates automatically, either by finding (more or less) exact chronological matches, or by interpolating a likely position if no exact match can be found.\
Coordinates can also be set or corrected manually by dragging and dropping images on the map directly. This always can be done, even if no geodata is available.

### Fixing a time drift

The clocks of cameras mostly aren't radio-controlled and often have a slight offset. If the images' dates have a time drift due to the camera's clock being not exactly in sync with the GPS data (which is assumed to be correct), a deviation can be defined. It then will be considered when searching for matches.

### Optionally: Adding elevation information

The elevation of manually assigned coordinates or already tagged images lacking an altitude value can be looked up using [opentopodata.org](https://www.opentopodata.org/)'s API (disabled by default).

### Making the data persistent

Finally, the assigned coordinates can be saved in the images' Exif header, making them persistent and also accessible for other geodata-aware applications (e. g. [KPhotoAlbum](https://www.kphotoalbum.org/)). If a time drift has been identified and a deviation has been given, the images' dates and times also can be fixed whilst saving.
