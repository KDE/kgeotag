// SPDX-FileCopyrightText: 2024 Tobias Leupold <tl at stonemx dot de>
//
// SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#ifndef DEGREESCONVERTER_H
#define DEGREESCONVERTER_H

namespace DegreesConverter
{

double fromDecimal(double value, double *minutes);
double fromDecimal(double value, double *minutes, double *seconds);

double toDecimal(double degrees, double minutes);
double toDecimal(double degrees, double minutes, double seconds);

}

#endif // DEGREESCONVERTER_H
