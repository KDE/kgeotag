/* Copyright (C) 2020 Tobias Leupold <tobias.leupold@gmx.de>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of
   the License or (at your option) version 3 or any later version
   accepted by the membership of KDE e. V. (or its successor approved
   by the membership of KDE e. V.), which shall act as a proxy
   defined in Section 14 of version 3 of the license.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

// Local includes
#include "ImageItem.h"

// KDE includes
#include <KColorScheme>

ImageItem::ImageItem(const QIcon &icon, const QString &fileName, const QString &path,
                     const QDateTime &date)
    : QListWidgetItem(icon, fileName),
      m_fileName(fileName),
      m_path(path),
      m_date(date)
{
}

QString ImageItem::path() const
{
    return m_path;
}

QDateTime ImageItem::date() const
{
    return m_date;
}

void ImageItem::setChanged(bool state)
{
    setText(state ? QStringLiteral("*") + m_fileName : m_fileName);
}

void ImageItem::setMatchType(KGeoTag::MatchType matchType)
{
    KColorScheme scheme;
    switch (matchType) {
    case KGeoTag::MatchType::None:
        setForeground(scheme.foreground());
        break;
    case KGeoTag::MatchType::Exact:
        setForeground(scheme.foreground(KColorScheme::PositiveText));
        break;
    case KGeoTag::MatchType::Interpolated:
        setForeground(scheme.foreground(KColorScheme::NeutralText));
        break;
    case KGeoTag::MatchType::Set:
        setForeground(scheme.foreground(KColorScheme::LinkText));
        break;
    }
}

bool ImageItem::operator<(const QListWidgetItem &other) const
{
    return m_date < dynamic_cast<const ImageItem *>(&other)->date();
}
