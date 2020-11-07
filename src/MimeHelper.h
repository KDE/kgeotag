/*
    SPDX-FileCopyrightText: 2020 Tobias Leupold <tobias.leupold@gmx.de>

    SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef MIMEHELPER_H
#define MIMEHELPER_H

// Local includes
#include "KGeoTag.h"

// Qt includes
#include <QString>
#include <QVector>

// Qt classes
class QMimeData;

namespace MimeHelper
{

QVector<QString> getUsablePaths(KGeoTag::DropTarget dropTarget, const QMimeData *data);
bool mimeTypeOkay(const QString &path);
QString mimeType(const QString &path);

}

#endif // MIMEHELPER_H
