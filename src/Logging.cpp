// SPDX-FileCopyrightText: 2022 Tobias Leupold <tl at stonemx dot de>
//
// SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

// Local includes
#include "Logging.h"
#include "debugMode.h"

#ifdef DEBUG_MODE
// Enable all messages in debug mode
Q_LOGGING_CATEGORY(KGeoTagLog, "KGeoTag")
#else
// Only enable warnings otherwise
Q_LOGGING_CATEGORY(KGeoTagLog, "KGeoTag", QtWarningMsg)
#endif
