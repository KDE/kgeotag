/* SPDX-FileCopyrightText: 2021 Tobias Leupold <tobias.leupold@gmx.de>

   SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-KDE-Accepted-GPL
*/

// Local includes
#include "TracksListView.h"
#include "GeoDataModel.h"

TracksListView::TracksListView(GeoDataModel *model, QWidget *parent) : QListView(parent)
{
    setModel(model);
}
