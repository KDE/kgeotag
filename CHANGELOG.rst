.. SPDX-FileCopyrightText: 2021-2022 Tobias Leupold <tl at stonemx dot de>

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

* KGeoTag now supports handling of some TIFF-based RAW image formats (cr2, nef and dng). By default,
  XMP sidecar files are created for those (regardless of the global setting). Additionally, there's
  a new option for enabling direct Exif header changes for RAW files.

Changed
=======

Deprecated
==========

Removed
=======

Fixed
=====

* IANA's timezone data 2022c renamed ``Europe/Kiev`` to ``Europe/Kyiv``. Currently, our timezone
  boundary data file is based on 2021c, and not many distributions already adopted 2022c. Thus, if
  the distribution already has ``Europe/Kyiv``, ``Europe/Kiev`` is now mapped accordingly.

Security
========

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
