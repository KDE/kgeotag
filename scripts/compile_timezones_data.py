#!/usr/bin/env python
""" SPDX-FileCopyrightText: 2021 Isaac Wismer <isaac@iwismer.ca>

    SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-KDE-Accepted-GPL

    This script takes a timezone data file (designed for the ones providede
    here: https://github.com/evansiroky/timezone-boundary-builder/), and turns it
    into a PNG with each of the timezones a unique color, and a JSON file with a
    mapping between the color and timezone.

    This script can be run with the following command:
    python timezone-png-creator [path-to-datafile] [output-dir]
    the --height flag can be used to change the height of the image.

    This script requires QGIS to be installed on the machine, and currently only
    works on Linux, but with a few small tweaks could work on Windows as well.
"""

import argparse
import json
import tempfile
from random import randrange
from pathlib import Path
from typing import Dict
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


def stylize_map(layer: QgsVectorLayer) -> Dict[str, str]:
    """Stylize the layer with random colors.

    Args:
        layer (QgsVectorLayer): The layer to stylize

    Returns:
        Dict[str, str]: The mapping of color to timezone.
    """

    print("Reading timezones from file")
    timezones = layer.uniqueValues(layer.fields().indexOf("tzid"))

    categorized_renderer = QgsCategorizedSymbolRenderer()

    print("Stylizing map")
    mapping = {}
    features = layer.getFeatures()
    categories = []
    for tz in timezones:
        # Modify the Etc timezones to match the Qt format.
        # There are a few exceptions where things don't quite match up.
        qt_tz = tz
        match = re.match(r"Etc/GMT([+-])(\d+)", tz)
        if match:
            qt_tz = f"UTC{match.group(1)}{match.group(2):0>2}:00"
        elif tz == "Etc/UTC":
            qt_tz = "UTC"
        elif tz == "Etc/GMT":
            qt_tz = "UTC+00:00"
        # Give random colors to the timezones, but ensure there are no duplicates.
        while True:
            r = randrange(0, 256)
            g = randrange(0, 256)
            b = randrange(0, 256)
            # Need to remove the leading '0x' so use [2:]
            rh = hex(r)[2:]
            gh = hex(g)[2:]
            bh = hex(b)[2:]
            hex_color = f"#{rh:0>2}{gh:0>2}{bh:0>2}".upper()
            if hex_color not in mapping:
                mapping[hex_color] = qt_tz
                break
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
    return mapping


def export_data(layer: QgsVectorLayer, mapping: Dict[str, str], path: Path,
                image_height: int) -> None:
    """Saves the image and mapping file.

    Args:
        layer (QgsVectorLayer): The layer to save.
        mapping (Dict[str, str]): The mapping dictionary.
        path (Path): The folder to save the data to.
        image_height (int): The height of the image to save.
    """

    path.mkdir(parents=True, exist_ok=True)

    json_file = (path / "timezones.json").resolve()
    print(f"Saving mappings JSON file to: {json_file.absolute()}")
    with open(json_file, "w") as f:
        f.write(json.dumps(mapping))

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
        """Function to save the rendered mao once it is done rendering."""
        img = render.renderedImage()
        img.save(str(png_file), "png")

    render = QgsMapRendererParallelJob(settings)
    render.finished.connect(finished)
    render.start()
    # This ensures that the program doesn't exit before the image is saved.
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
                        default=1000)
    args = vars(parser.parse_args())

    print(f"Opening data file: {args['input-timezone-data'].absolute().resolve()}")
    layer = QgsVectorLayer(str(args["input-timezone-data"]))
    mapping = stylize_map(layer)
    export_data(layer, mapping, args["outdir"], args["height"])


if __name__ == "__main__":
    main()
