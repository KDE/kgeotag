// SPDX-FileCopyrightText: 2026 Tobias Leupold <tl@stonemx.de>
//
// SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#ifndef SEARCHPLACESWIDGET_H
#define SEARCHPLACESWIDGET_H

// Qt includes
#include <QWidget>

// Local classes
class SharedObjects;
class MapWidget;

// Marble classes
namespace Marble
{
class SearchRunnerManager;
class GeoDataPlacemark;
}

// Qt classes
class QLineEdit;
class QListWidget;
class QListWidgetItem;

class SearchPlacesWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SearchPlacesWidget(SharedObjects *sharedObjects, QWidget *parent = nullptr);

private Q_SLOTS:
    void startSearch();
    void showResults(const QList<Marble::GeoDataPlacemark *> &results);
    void centerMap(QListWidgetItem *item);

private: // Variables
    MapWidget *m_mapWidget;
    Marble::SearchRunnerManager *m_searchRunnerManager;
    QLineEdit *m_searchTerm;
    QListWidget *m_results;

};

#endif // SEARCHPLACESWIDGET_H
