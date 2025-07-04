.. SPDX-FileCopyrightText: 2021-2025 Tobias Leupold <tl@stonemx.de>

   SPDX-License-Identifier: CC-BY-SA-4.0


   The format of this file is inspired by keepachangelog.com, but uses ReStructuredText instead of
   MarkDown. Keep the line length at no more than 100 characters (with the obvious exception of the
   header template below, which needs to be indented by three spaces)

   Here's the header template to be pasted at the top after a new release:

   ====================================================================================================
   [unreleased]
   ====================================================================================================

   Added
   =====

   * for new features.

   Changed
   =======

   * for changes in existing functionality.

   Deprecated
   ==========

   * for soon-to-be removed features.

   Removed
   =======

   * for now removed features.

   Fixed
   =====

   * for any bug fixes.

   Security
   ========

   * in case of vulnerabilities.

====================================================================================================
[unreleased]
====================================================================================================

Added
=====

Changed
=======

* Updated the timezones data files to 2025b (cf. Timezone Boundary Builder's `Release Announcement
  <https://github.com/evansiroky/timezone-boundary-builder/releases/tag/2025b>`_).

Deprecated
==========

Removed
=======

* As current distributions don't ship the needed dependencies anymore, the support for Qt 5 and KF 5
  has been dropped. KGeoTag is now Qt6/KF6-only.

Fixed
=====

Security
========

====================================================================================================
KGeoTag 1.8.0 released (2025-03-09)
====================================================================================================

Added
=====

* The current map center's coordinates can now be exported in the decimal degree "latitude,
  longitude" format to be used e.g. in Google Maps or OpenStreetMap.

* It's now possible to not only assign coordinates from the clipboard that have been copied from
  OpenStreetMap or Google Maps, but also using the human-readable format that KGeoTag uses when
  selecting "Copy formatted coordinates" for the current map center.

Changed
=======

* Updated the timezones data files to 2025a (cf. Timezone Boundary Builder's `Release Announcement
  <https://github.com/evansiroky/timezone-boundary-builder/releases/tag/2025a>`_).

Fixed
=====

* Assigning an image to coordinates from the clipboard now also works if the coordinates are
  negative. Additionally, we can now also parse our own human-readable format (provided the
  coordinates string has been formatted using the same locale as is used to parse it).
  This fixes `Bug #497410 <https://bugs.kde.org/show_bug.cgi?id=497410>`_.

====================================================================================================
KGeoTag 1.7.0 released (2024-12-07)
====================================================================================================

Changed
=======

* KGeoTag can now be compiled against Qt5/KF5 as well as Qt6/KF6. If Qt 5 is found, a Qt5/KF5 build
  is done. If only Qt 6 is found, a Qt6/KF6 build is done respectively. In a mixed environment with
  both Qt 5 and 6 being present, a Qt6/KF6 build can be forced by setting ECM's ``BUILD_WITH_QT6``
  variable, i.e. passing :code:`-DBUILD_WITH_QT6=ON` to cmake.

* Made ``compile_timezones_data.py`` work with QGIS 3.38 and Qt 6.

====================================================================================================
KGeoTag 1.6.0 released (2024-10-03)
====================================================================================================

Added
=====

* It's now possible to (re-)assign all currently selected images to the current map center using a
  keyboard shortcut. Optionally, you can also have the next untagged image selected automatically
  afterwards. Thanks to Robert Menger for the respective feature request :-)

* Added more functionality to the map's context menu: Bookmarks can now be added for the current
  map center and it's coordinates can be copied to the clipboard.

* The displayed order of coordinates can now be configured, either "Longitude, latitude" (as
  before) or "Latitude, longitude".

* One can now choose the coordinates "flavor": Either decimal degrees (as before), degrees and
  decimal minutes or degrees, minutes and decimal seconds.

Changed
=======

* Updated the timezones data files to 2024b (cf. Timezone Boundary Builder's `Release Announcement
  <https://github.com/evansiroky/timezone-boundary-builder/releases/tag/2024b>`_).

Fixed
=====

* Fixed a crash when removing a GPX track and moving the "track walker" slider afterwards. Whilst
  doing so, also fixed the track points counting for slider when removing a GPX track and loading
  another one directly afterwards.

* Changes to the image list layout (combined/separate) are now displayed correctly (again).

* Restored compatibility with Ubuntu LTS 20.04.

* The window state saved by the KXmlGuiWindow/KMainWindow class is now used to restore the dock
  arrangement (or setup the default one) instead of saving the saveState() twice, only using a
  different serialization.

* The last window position is now restored correctly when closing the program and opening it again.

* Closed floating docks are now properly restored by using "Set default dock arrangement". This
  addresses bug #488597, but actually, it's an upstream Qt bug (reported as
  `bug #126418 <https://bugreports.qt.io/browse/QTBUG-126418>`_), as the docks shouldn't be closable
  at all in the first place.

====================================================================================================
KGeoTag 1.5.0 released (2024-01-02)
====================================================================================================

Added
=====

* It's now possible to find the chronologically closest track point for an unassigned image and
  center the map there. If an image has been taken in the time between two tracks, one can find the
  last or first coordinates of the track recorded before or afterwards (whichever is closer) and use
  them as a basis for manual tagging.

Changed
=======

* Updated the timezones data files to 2023d (cf. Timezone Boundary Builder's `Release Announcement
  <https://github.com/evansiroky/timezone-boundary-builder/releases/tag/2023d>`_).

====================================================================================================
KGeoTag 1.4.0 released (2023-01-28)
====================================================================================================

Added
=====

* KGeoTag now supports handling of some TIFF-based RAW image formats (cr2, nef and dng). By default,
  XMP sidecar files are created for those (regardless of the global setting). Additionally, there's
  a new option for enabling direct Exif header changes for RAW files.

Changed
=======

* Bumped the dependencies to what is still provided by Ubuntu 20.04 LTS: KF 5.68.0 and Qt 5.12.0.
  We already had a dependecy bump of CMake to 3.16.0 earlier. Debian Bullseye even has more recent
  versions, so we should be quite safe with this.

* The dependency bump to `Marble <https://marble.kde.org/>`_ 21.12.0 broke the compatibility with
  (at least) Ubuntu LTS 20.04 and Debian Bullseye. As KGeoTag could always be compiled against
  Marble from the beginning, the required Marble version is now omitted again, until >= 21.12.0 hits
  LTS distros.

* Updated the timezones data files to 2022g (cf. Timezone Boundary Builder's `Release Announcement
  <https://github.com/evansiroky/timezone-boundary-builder/releases/tag/2022g>`_).

* The compile_timezones_data.py script now derives timezone colors from the timezone name rather
  than counting a color up on a sorted list. This should minimalize changes to the timezones.json
  file (smaller diffs on updates). As a side-effect, we now have nicer colors in timezones.png ;-)

Fixed
=====

* KGeoTag can now be built against Qt 5.12 (again), and thus also (again) on Ubuntu LTS 20.04.

====================================================================================================
KGeoTag 1.3.1 released (2022-09-05)
====================================================================================================

Added
=====

* When using the split images list view and tagging images manually, the "assigned" image list now
  scrolls to the last (or only) tagged image and highlights it, so that it's immediately accessible
  for e.g. further corrections.

Fixed
=====

* Due to a problem with the tarme.rb releasme script, the 1.3.0 tarball lacked all l10n data. We
  thus had to withdraw the release and tag a new one.

====================================================================================================
KGeoTag 1.3.0 released (2022-09-04)
====================================================================================================

Added
=====

* Allow setting coordinates directly from the clipboard. As of now, Google Maps' format as well as
  OpenStreetMap's Geo URI scheme is supported (implements feature request #458537).

* It is now possible to open an image with the system's default image viewer (for closer
  inspection), either from the image list(s), or from the preview widget.

* Added a handbook stub (implementes feature request/bug #452534).

* Files and/or directories given on the command line are now loaded after startup (which can e.g.
  also be triggered via "Open with" from a file manager like Dolphin).

Changed
=======

* Updated the timezones data files to 2021c (cf. Timezone Boundary Builder's `Release Announcement
  <https://github.com/evansiroky/timezone-boundary-builder/releases/tag/2021c>`_).

* As `Marble <https://marble.kde.org/>`_ now has a decent versioning scheme (at least since the KDE
  Apps release 21.12.3), KGeoTag now has a defined dependecy for Marble: It now depends on at least
  Marble 21.12.0. It actually can be built against older versions as well, but due to the lack of
  proper version bumps, there was no way to define a definitive version to depend on until now.

Fixed
=====

* Use "Folder", not "Directory" according to KDE's style/vocabulary guidelines (fixes bug #457020).

====================================================================================================
KGeoTag 1.2.0 released (2021-11-12)
====================================================================================================

Added
=====

* KGeoTag now checks if the timezone data files could actually be loaded and displays a warning
  about timezone detection not working if not.

Changed
=======

* Suppressed a warning by bumping :code:`cmake_minimum_required` to :code:`3.16.0`.

Fixed
=====

* Made selecting all entries of an image list work again via the standard shortcut ``CTRL+A``. This
  one was assigned to "Assign images to GPS data", which can now be accessed via ``CTRL+M`` (fixes
  bug #445023).

* :code:`QMimeDatabase::mimeTypeForFile` now returns ``application/xml+gpx`` as a GPX file's MIME
  type instead of ``application/x-gpx+xml`` – at least here on my stable Gentoo machine. This made
  KGeoTag refuse to load any GPX file. Now, both MIME types are accepted.

====================================================================================================
KGeoTag 1.1.0 released (2021-10-16)
====================================================================================================

Added
=====

* Added an "Assign images to GPS data" main menu entry. This one triggers an automatic image
  assignment of all loaded images, like "(Re)Assign all images" from the "Automatic assignment"
  dock. The search type to perform can be defined in the settings.

Fixed
=====

* When walking along a track, the date and time were displayed to be in the set timezone, but not
  actually converted to it. Now, the timestamps are translated correctly.

* Enabled exact matching also if a used GPX track and/or the image metadata contain milliseconds. If
  so, the :code:`QHash<QDateTime, ...>::contains()` call used to search for exact matches won't
  yield a result, even if there's a seconds-exact match (if not also the milliseconds matched, and
  this is quite unlikely).

====================================================================================================
KGeoTag 1.0.0 released (2021-02-25)
====================================================================================================

* First official release
