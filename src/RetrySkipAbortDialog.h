// SPDX-FileCopyrightText: 2020 Tobias Leupold <tl at l3u dot de>
//
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-KDE-Accepted-GPL

#ifndef RETRYSKIPABORTDIALOG_H
#define RETRYSKIPABORTDIALOG_H

// Qt includes
#include <QMessageBox>

class RetrySkipAbortDialog : public QMessageBox
{
    Q_OBJECT

public:
    enum Reply {
        Retry,
        Skip,
        Abort
    };

    explicit RetrySkipAbortDialog(QWidget *parent, const QString &title, const QString &text,
                                  bool isSingleFile);
    int exec() override;

};

#endif // RETRYSKIPABORTDIALOG_H
