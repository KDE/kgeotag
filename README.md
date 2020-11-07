# KGeoTag

**Warning**: KGeoTag is currently in the beta phase. It's feature-complete and should work fine, but version 1.0.0 has not been tagged yet. All beta testing and feedback is greatly appreciated!

## What is it?

KGeoTag is a Free/Libre Open Source photo geotagging program. It's written in C++/[Qt](https://www.qt.io/) and uses the [KDE Frameworks](https://api.kde.org/frameworks/). It's published under the terms of the [GNU General Public License (GPL)](https://www.gnu.org/licenses/#GPL).

## What is "geotagging"?

JPEG images contain metadata (e. g. the creation date, camera information etc.). Those are either stored in the so-called [Exif header](https://en.wikipedia.org/wiki/Exif), in an [XMP sidecar file](https://en.wikipedia.org/wiki/Extensible_Metadata_Platform) or in both. This data can also represent geographic coordinates so that it's replicable where the images were taken.

Most cameras don't have GPS receivers, so, most can't save coordinates when taking images. A common approach is to e. g. carry a small GPS logging device along, which records a track all the time. Later on, the images' dates can be compared to the GPS log's points' dates to figure out where an image was taken.

If one knows for sure where the respective photo was taken, it's also possible to assign coordinates to the images manually.

## Geotagging with KGeoTag

### Assigning images

KGeoTag offers multiple ways to assign coordinates to images. The interface provides a map view that can be used to display geodata that can be used for tagging, either automatical or manual. Completely manual tagging (without any geodata) is also possible.

#### Automatic tagging

The most convenient approach is automatic assignment. Geodata stored in the [GPX](https://en.wikipedia.org/wiki/GPS_Exchange_Format) format can be loaded and be displayed on the map. Using this data, images can be matched with their respective geographic coordinates, either by finding (more or less) exact chronological matches, or by interpolating a likely position if no exact match can be found. Of course, this needs some GPX file (e. g. from a logger) to be available.

If altitude values aren't set in the dataset or they are not accurate enough, they can be looked up using opentopodata.org (see below).

The clocks of cameras mostly aren't radio-controlled and often have a slight offset. If the images' dates have a time drift due to the camera's clock being not exactly in sync with the GPS data (which is assumed to be correct), a deviation can be defined. It then will be considered when searching for matches, and can also be used to fix the images' dates.

#### Drag and drop interface

Another option is tagging images via drag and drop. One or more images can be selected and dropped onto the map, to their respective location (possibly guided by some GPX track). Elevations can't be ascertained this way, so either, they are left to be "0 m" (sea level), entered manually or looked up via opentopodata.org (either automatically or manually, see below).

#### Assigning images to bookmarks

For places frequently assigned to images (like one's home, where one doesn't carry a GPS logger), bookmarks can be defined. Images can then be assigned to the respective coordinates easily.

#### Manual assignment

It's also possible to enter coordinates for one or more images by hand. The altitudes can be either looked up automatically using opentopodata.org (see below), or also be entered manually.

### Looking up (missing or unprecise) elevation information

If a manual assignment is done, or the geodata used for automatic matching lacks altitudes, or the elevation information is not precise enough, the altitudes can be looked up querying different elevation datasets using [opentopodata.org](https://www.opentopodata.org/)'s API.

By default, this is turned off. If switched on, by default, the [ASTER](https://asterweb.jpl.nasa.gov/gdem.asp) dataset is used. This one covers the whole globe. Others can be used as well, cf. [opentopodata.org's homepage](https://www.opentopodata.org/#public-api). The respective altitude is then looked up in the background and set as soon as the server answers when dropping images onto the map or entering coordinates manually.

Optionally, the altitude levels can also be entered by hand. Existing altitude values can also be overwritten by a manual server lookup.

### Making the data persistent

Finally, the assigned coordinates can be saved. KGeoTag can either write them to the images' Exif header (which will obviously alter the respective files), to XMP sidecar files (leaving the original files untouched) or to both. By default, a backup of each original file is created if it will be changed during the write process.

This way, the geodata assignment is made persistent and also accessible for other geodata-aware applications (like e. g. [KPhotoAlbum](https://www.kphotoalbum.org/)).

If a time drift has been identified and a deviation has been given, the images' dates and times also can be fixed whilst saving.
