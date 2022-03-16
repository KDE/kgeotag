#!/usr/bin/env python

# SPDX-FileCopyrightText: 2021-2022 Isaac Wismer <isaac@iwismer.ca>
#
# SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-KDE-Accepted-GPL

"""
    This script takes a timezone data file (designed for the ones providede
    here: https://github.com/evansiroky/timezone-boundary-builder/), and turns it
    into a PNG with each of the timezones a unique color, and a JSON file with a
    mapping between the color and timezone.

    This script can be run with the following command:
    python timezone-png-creator <path-to-datafile> [output-dir]
    the --height flag can be used to change the height of the image.

    This script requires QGIS to be installed on the machine, and currently only
    works on Linux, but with a few small tweaks could work on Windows as well.
"""

import argparse
import tempfile
from pathlib import Path
from typing import List
import re

from PyQt5.QtCore import QSize, Qt
from PyQt5.QtGui import QColor
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
    currentColor = 0

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

        # Generate a consecutive color for each timezone
        currentColor += 25000
        r = (currentColor >> 16) & 255
        g = (currentColor >> 8 ) & 255
        b = (currentColor      ) & 255

        # Add it to the mapping
        rh = hex(r)[2:]
        gh = hex(g)[2:]
        bh = hex(b)[2:]
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
    # Turn antialiasing off
    settings.setFlag(1, False)

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
    loop.exec_()

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("input-timezone-data",
                        type=Path,
                        help="The timezone data file.")
    parser.add_argument("--outdir",
                        type=Path,
                        help="The folder to place the output data files in.",
                        default=".")
    parser.add_argument("--height",
                        type=int,
                        help="The height of the output image. Should be an even number.",
                        default=2000)
    args = vars(parser.parse_args())

    print(f"Opening data file: {args['input-timezone-data'].absolute().resolve()}")
    layer = QgsVectorLayer(str(args["input-timezone-data"]))
    timezone_ids, timezone_colors = stylize_map(layer)
    export_data(layer, timezone_ids, timezone_colors, args["outdir"], args["height"])

if __name__ == "__main__":
    main()
