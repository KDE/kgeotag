#!/usr/bin/env python

# SPDX-FileCopyrightText: 2021-2023 Isaac Wismer <isaac@iwismer.ca>
# SPDX-FileCopyrightText: 2024 Tobias Leupold <tl at stonemx dot de>
#
# SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

"""
    This script takes a timezone data file from the Timezone Boundary Builder project
    (cf. https://github.com/evansiroky/timezone-boundary-builder/), and turns it into a PNG image
    with each of the timezones a unique color, and a JSON file with a mapping between the color and
    timezone.

    By default, "combined-shapefile-with-oceans.shp" is used as the shapefile input, as provided by
    Timezone Boundary Builder's "timezones-with-oceans.shapefile.zip" download.

    The image height, output dir and shapefile input can be adjusted, cf. the --help message.

    This script requires QGIS to be installed on the machine, and is currently only tested on Linux.
    However, with a few tweaks, it could most probably work on Windows as well.
"""

import argparse
import tempfile
from pathlib import Path
from typing import List
import re
from hashlib import sha1
import sys

from PyQt6.QtCore import QSize, Qt
from PyQt6.QtGui import QColor
from qgis.core import (
    QgsApplication,
    QgsCategorizedSymbolRenderer,
    QgsMapRendererParallelJob,
    QgsMapSettings,
    QgsRendererCategory,
    QgsSimpleFillSymbolLayer,
    QgsSymbol,
    QgsVectorLayer,
)
from qgis.PyQt.QtCore import QEventLoop

# Initialize QGis
qgs = QgsApplication([], False)
QgsApplication.setPrefixPath("/usr", True)
QgsApplication.initQgis()

def stylize_map(layer: QgsVectorLayer) -> [List[str], List[str]]:
    """Stylize the layer with unique colors per timezone

    Args:
        layer (QgsVectorLayer): The layer to stylize

    Returns:
        [List[str], List[str]]: A list with all timezone ids and one with the respective color
    """

    print("Reading timezones from file")
    timezones = layer.uniqueValues(layer.fields().indexOf("tzid"))
    timezones = list(timezones)
    timezones.sort()

    categorized_renderer = QgsCategorizedSymbolRenderer()

    print("Stylizing map")

    timezone_ids = []
    timezone_colors = []
    features = layer.getFeatures()
    categories = []
    usedColors = []

    for tz in timezones:
        # Modify the Etc timezones to match the Qt format

        qt_tz = tz

        # There are a few exceptions where the Qt timezone ids differ from the dataset ids:
        match = re.match(r"Etc/GMT([+-])(\d+)", tz)
        if match:
            qt_tz = f"UTC{match.group(1)}{match.group(2):0>2}:00"
        elif tz == "Etc/UTC":
            qt_tz = "UTC"
        elif tz == "Etc/GMT":
            qt_tz = "UTC+00:00"

        # Derive a color from the timezone's name

        hex = sha1(qt_tz.encode("utf-8")).hexdigest()[0:6]
        if hex in usedColors:
            # This is very unlikely if not impossible to happen, but who knows?!
            print("Timezone {} caused a color collision! Please review this script!".format(qt_tz))
            sys.exit(1)
        usedColors.append(hex)

        rh = hex[0:2]
        gh = hex[2:4]
        bh = hex[4:6]

        r = int(rh, 16)
        g = int(gh, 16)
        b = int(bh, 16)

        # Add it to the mapping
        timezone_ids.append(qt_tz)
        timezone_colors.append(f"#{rh:0>2}{gh:0>2}{bh:0>2}")

        symbol = QgsSymbol.defaultSymbol(layer.geometryType())
        symbol_layer = QgsSimpleFillSymbolLayer.create({"color": f"{r}, {g}, {b}"})
        symbol_layer.setStrokeWidth(0.0)
        symbol_layer.setStrokeStyle(Qt.PenStyle.NoPen)
        symbol.changeSymbolLayer(0, symbol_layer)

        category = QgsRendererCategory(tz, symbol, tz)
        categories.append(category)

    renderer = QgsCategorizedSymbolRenderer("tzid", categories)
    layer.setRenderer(renderer)
    layer.triggerRepaint()

    return timezone_ids, timezone_colors

def export_data(layer: QgsVectorLayer, timezone_ids: List[str], timezone_colors: List[str],
                path: Path, image_height: int) -> None:
    """Saves the image and mapping file

    Args:
        layer (QgsVectorLayer): The layer to save
        timezone_ids (List[str]): A list of all timezone ids
        timezone_colors (List[str]): A list of all timezone colors
        path (Path): The folder to save the data to
        image_height (int): The height of the image to save
    """

    path.mkdir(parents=True, exist_ok=True)

    # We write the JSON dataset by hand, so that the order of all key -> value mappings inside the
    # file is consistent. Using JSON functions, the dictionary would be written to the file in a
    # random order, making content versioning hard as the file would completely change each time
    # it is generated.
    json_file = (path / "timezones.json").resolve()
    print(f"Saving mappings JSON file to: {json_file.absolute()}")
    with open(json_file, "w") as f:
        f.write("{\n")
        last = len(timezone_ids)
        for i in range(0, last):
            f.write("\"{}\": \"{}\"".format(timezone_colors[i],  timezone_ids[i]))
            if i < last - 1:
                f.write(",")
            f.write("\n")
        f.write("}\n")

    png_file = (path / "timezones.png").resolve()
    print(f"Saving PNG map to: {png_file.absolute()}")
    settings = QgsMapSettings()
    settings.setLayers([layer])
    settings.setBackgroundColor(QColor(255, 255, 255))
    settings.setOutputSize(QSize(image_height * 2, image_height))
    settings.setExtent(layer.extent())
    settings.setFlag(QgsMapSettings.Antialiasing, False)

    def finished() -> None:
        """Function to save the rendered map once it is done rendering"""
        img = render.renderedImage()
        img.save(str(png_file), "png")

    render = QgsMapRendererParallelJob(settings)
    render.finished.connect(finished)
    render.start()

    # This ensures that the program doesn't exit before the image is saved
    loop = QEventLoop()
    render.finished.connect(loop.quit)
    loop.exec()

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--shapefile",
                        type = Path,
                        help = "The timezone data shapefile (.shp, defaults to "
                               "combined-shapefile-with-oceans.shp)",
                        default = "combined-shapefile-with-oceans.shp")
    parser.add_argument("--outdir",
                        type = Path,
                        help = "The folder to place the output data files in (defaults to .)",
                        default = ".")
    parser.add_argument("--height",
                        type = int,
                        help = "The height of the output image. Should be an even number "
                               "(defaults to 2000)",
                        default = 2000)
    args = vars(parser.parse_args())

    print(f"Opening data file: {args['shapefile'].absolute().resolve()}")
    layer = QgsVectorLayer(str(args["shapefile"]))
    timezone_ids, timezone_colors = stylize_map(layer)
    export_data(layer, timezone_ids, timezone_colors, args["outdir"], args["height"])

if __name__ == "__main__":
    main()
