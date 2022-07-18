// SPDX-FileCopyrightText: 2020-2021 Tobias Leupold <tl at stonemx dot de>
//
// SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

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
QString mimeType(const QString &path);
KGeoTag::FileType classifyFile(const QString &path);

}

#endif // MIMEHELPER_H
