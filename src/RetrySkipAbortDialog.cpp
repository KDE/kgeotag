// SPDX-FileCopyrightText: 2020 Tobias Leupold <tl at l3u dot de>
//
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-KDE-Accepted-GPL

// Local includes
#include "RetrySkipAbortDialog.h"

// KDE includes
#include <KLocalizedString>

// Qt includes
#include <QAbstractButton>

RetrySkipAbortDialog::RetrySkipAbortDialog(QWidget *parent, const QString &title,
                                           const QString &text, bool isSingleFile)
    : QMessageBox(parent)
{
    setIcon(QMessageBox::Warning);
    setWindowTitle(title);

    if (isSingleFile) {
        setStandardButtons(QMessageBox::Retry | QMessageBox::Abort);
    } else {
        setStandardButtons(QMessageBox::Retry | QMessageBox::Discard | QMessageBox::Abort);
        button(QMessageBox::Discard)->setText(i18n("Skip current image"));
    }

    setText(text);
}

int RetrySkipAbortDialog::exec()
{
    const auto reply = QMessageBox::exec();
    if (reply == QMessageBox::Discard) {
        return Reply::Skip;
    } else if (reply == QMessageBox::Abort) {
        return Reply::Abort;
    } else {
        return Reply::Retry;
    }
}
