# KGeoTag

If you are reading this on GitHub, be aware that this is just a mirror. The real code repository can be found at https://invent.kde.org/graphics/kgeotag.

## What is it?

KGeoTag is a Free/Libre Open Source photo geotagging program. It's written in C++/[Qt](https://www.qt.io/) and uses the [KDE Frameworks](https://api.kde.org/frameworks/). It's published under the terms of the [GNU General Public License (GPL)](https://www.gnu.org/licenses/#GPL).

## What is "geotagging"?

Photos (e. g. JPEG images) contain metadata like the creation date, camera information etc. Those are either stored in the so-called [Exif header](https://en.wikipedia.org/wiki/Exif), in an [XMP sidecar file](https://en.wikipedia.org/wiki/Extensible_Metadata_Platform) or in both. This data can also represent geographic coordinates so that it's replicable where the images were taken.

Most cameras don't have GPS receivers, so, most can't save coordinates when taking images. A common approach is to e. g. carry a small GPS logging device along, which records a track all the time. Later on, the images' dates can be compared to the GPS log's points' dates to figure out where an image was taken.

If one knows for sure where the respective photo was taken, it's also possible to assign coordinates to the images manually.

## Geotagging with KGeoTag

### Supported file formats

KGeoTag currently supports The following image formats: [JPEG](https://en.wikipedia.org/wiki/JPEG), [PNG](https://en.wikipedia.org/wiki/Portable_Network_Graphics), [WebP](https://en.wikipedia.org/wiki/WebP), [TIFF](https://en.wikipedia.org/wiki/TIFF), [OpenRaster](https://en.wikipedia.org/wiki/OpenRaster) and [Krita Document](https://en.wikipedia.org/wiki/Krita).

### Assigning images

KGeoTag offers multiple ways to assign coordinates to images:

#### Automatic tagging

The most convenient approach is automatic matching with geodata stored in the [GPX](https://en.wikipedia.org/wiki/GPS_Exchange_Format) format. GPX files can be loaded and displayed on a map. Using this data, images can be assigned with matching geographic coordinates by finding (more or less) exact chronological matches, or by interpolating a likely position if no exact match can be found.

If altitude values aren't set in the dataset or they are not accurate enough, they can be looked up using opentopodata.org (see below).

The clocks of cameras mostly aren't radio-controlled and often have a slight offset. If the images' dates have a time drift due to the camera's clock being not exactly in sync with the GPS data (which is assumed to be correct), a deviation can be defined. It then will be considered when searching for matches, and can also be used to fix the images' dates.

#### Drag and drop interface

Another option is tagging images via drag and drop. One or more images can be selected and dropped onto the map, to their respective location (possibly also guided by some GPX track). Elevations can't be ascertained this way, so either, they are left to be "0 m" (sea level), entered manually or looked up via opentopodata.org (either automatically or manually, see below).

#### Assigning images to bookmarks

For places frequently assigned to images (like one's home, where one doesn't carry a GPS logger), bookmarks can be defined. Images can then be assigned to the respective coordinates easily.

#### Manual assignment

It's also possible to enter coordinates for one or more images by hand. The altitudes can be either looked up automatically using opentopodata.org (see below), or also be entered manually.

### Setting or looking up elevation information

Altitudes can always be set maually. Alternatively, the altitudes can also be looked up querying different elevation datasets using [opentopodata.org](https://www.opentopodata.org/)'s API.

The preset it to use the [ASTER](https://asterweb.jpl.nasa.gov/gdem.asp) dataset. This one covers the whole globe. Others can be used as well, cf. [opentopodata.org's homepage](https://www.opentopodata.org/#public-api).

By default, such a server lookup has to be triggered manually. It's also possible to enable automated altitude lookups for all images dropped on the map (which yields geographic coordinates but no elevation) and coordinates entered manually.

### Making the data persistent

Finally, the assigned coordinates can be saved. KGeoTag can either write them to the images' Exif header (which will obviously alter the respective files), to XMP sidecar files (leaving the original files untouched) or to both. By default, a backup of each original file is created if it will be changed during the write process.

This way, the geodata assignment is made persistent and also accessible for other geodata-aware applications (like e. g. [KPhotoAlbum](https://www.kphotoalbum.org/)).

If a time drift has been identified and a deviation has been given, the images' dates and times also can be fixed whilst saving.

## Getting involved

Everybody is invited to participate in KGeoTag's development! You don't have to be able to write code to do so. All testing and feedback is greatly appreciated!

Also feel free to join KGeoTag's IRC channel [#kde-kgeotag](irc://chat.freenode.net/kde-kgeotag) :-)
