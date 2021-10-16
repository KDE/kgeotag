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
