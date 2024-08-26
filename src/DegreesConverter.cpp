// SPDX-FileCopyrightText: 2024 Tobias Leupold <tl at stonemx dot de>
//
// SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

// Local includes
#include "DegreesConverter.h"

// C++ includes
#include <cmath>

namespace DegreesConverter
{

double fromDecimal(double value, double *minutes)
{
    double degrees;
    const double decimals = std::modf(value, &degrees);
    *minutes = decimals * 60.0;
    return degrees;
}

double fromDecimal(double value, double *minutes, double *seconds)
{
    double decimals;
    double degrees;
    decimals = std::modf(value, &degrees);
    const double decimalMinutes = decimals * 60.0;
    decimals = std::modf(decimalMinutes, minutes);
    *seconds = decimals * 60.0;
    return degrees;
}

double toDecimal(double degrees, double minutes)
{
    return degrees + minutes / 60.0;
}

double toDecimal(double degrees, double minutes, double seconds)
{
    return degrees + minutes / 60.0 + seconds / 3600.0;
}

}
