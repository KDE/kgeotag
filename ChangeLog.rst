====================================================================================================
KGeoTag 1.2.0 released (12.11.2021)
====================================================================================================

* **Enhancement:** KGeoTag now checks if the timezone data files could actually be loaded and
  displays a warning about timezone detection not working if not.

* **Bugfix (#445023):** Made selecting all entries of an image list work again via the standard
  shortcut ``CTRL+A``. This one was assigned to "Assign images to GPS data", which can now be
  accessed via ``CTRL+M``.

* **Bugfix:** :code:`QMimeDatabase::mimeTypeForFile` now returns ``application/xml+gpx`` as a GPX
  file's MIME type instead of ``application/x-gpx+xml`` – at least here on my stable Gentoo machine.
  This made KGeoTag refuse to load any GPX file. Now, both MIME types are accepted.

* **Change:** Suppressed a warning by bumping :code:`cmake_minimum_required` to :code:`3.16.0`.

====================================================================================================
KGeoTag 1.1.0 released (16.10.2021)
====================================================================================================

* **Bugfix:** When walking along a track, the date and time were displayed to be in the set
  timezone, but not actually converted to it. Now, the timestamps are translated correctly.

* **Enhancement:** Added an "Assign images to GPS data" main menu entry. This one triggers an
  automatic image assignment af all loaded images, like "(Re)Assign all images" from the "Automatic
  assignment" dock. The search type to perform can be defined in the settings.

* **Bugfix:** Enabled exact matching also if a used GPX track and/or the image metadata contain
  milliseconds. If so, the :code:`QHash<QDateTime, ...>::contains()` call used to search for exact
  matches won't yield a result, even if there's a seconds-exact match (if not also the milliseconds
  matched, and this is quite unlikely).

====================================================================================================
KGeoTag 1.0.0 released (25.02.2021)
====================================================================================================

* First official release
