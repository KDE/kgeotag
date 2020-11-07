/*
    SPDX-FileCopyrightText: 2020 Tobias Leupold <tobias.leupold@gmx.de>

    SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef DROPHELPER_H
#define DROPHELPER_H

// Local includes
#include "KGeoTag.h"

// Qt includes
#include <QString>
#include <QVector>

// Qt classes
class QMimeData;

namespace DropHelper
{

QVector<QString> getUsablePaths(KGeoTag::DropTarget dropTarget, const QMimeData *data);

}

#endif // DROPHELPER_H
