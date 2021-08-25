* **Bugfix:** Enabled exact matching also if a used GPX track and/or the image metadata contain
  milliseconds. If so, the :code:`QHash<QDateTime, ...>::contains()` call used to search for exact matches
  won't yield a result, even if there's a seconds-exact match.

====================================================================================================
KGeoTag 1.0.0 released (25.02.2021)
====================================================================================================

* First official release
