// SPDX-FileCopyrightText: 2026 Tobias Leupold <tl@stonemx.de>
//
// SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

// Local includes
#include "SearchPlacesWidget.h"
#include "SharedObjects.h"
#include "MapWidget.h"
#include "Coordinates.h"

// Marble includes
#include <marble/SearchRunnerManager.h>
#include <marble/GeoDataPlacemark.h>

// KDE includes
#include <KLocalizedString>

// Qt includes
#include <QVBoxLayout>
#include <QLineEdit>
#include <QListWidget>
#include <QApplication>
#include <QToolButton>
#include <QHBoxLayout>
#include <QGroupBox>

SearchPlacesWidget::SearchPlacesWidget(SharedObjects *sharedObjects, QWidget *parent)
    : QWidget(parent),
      m_mapWidget(sharedObjects->mapWidget())
{
    m_searchRunnerManager = new Marble::SearchRunnerManager(m_mapWidget->model(), this);

    connect(m_searchRunnerManager, QOverload<const QList<Marble::GeoDataPlacemark *> &>::of(
                                       &Marble::SearchRunnerManager::searchResultChanged),
            this, &SearchPlacesWidget::showResults);

    auto *mainLayout = new QVBoxLayout(this);

    auto *searchBox = new QGroupBox(i18n("Search term"));
    auto *searchLayout = new QHBoxLayout(searchBox);
    mainLayout->addWidget(searchBox);

    m_searchTerm = new QLineEdit;
    connect(m_searchTerm, &QLineEdit::returnPressed, this, &SearchPlacesWidget::startSearch);
    searchLayout->addWidget(m_searchTerm);

    auto *searchButton = new QToolButton;
    searchButton->setIcon(QIcon::fromTheme(QStringLiteral("system-search")));
    searchButton->setToolTip(i18n("Start search"));
    searchLayout->addWidget(searchButton);
    connect(searchButton, &QToolButton::clicked, this, &SearchPlacesWidget::startSearch);

    auto *clearButton = new QToolButton;
    clearButton->setIcon(QIcon::fromTheme(QStringLiteral("zoom-original")));
    clearButton->setToolTip(i18n("Reset search"));
    searchLayout->addWidget(clearButton);
    connect(clearButton, &QToolButton::clicked, this, [this]
    {
        m_searchTerm->clear();
        startSearch();
        m_results->setEnabled(false);
    });

    auto *resultsBox = new QGroupBox(i18n("Search results"));
    auto *resultsLayout = new QVBoxLayout(resultsBox);
    mainLayout->addWidget(resultsBox);

    m_results = new QListWidget;
    connect(m_results, &QListWidget::itemClicked, this, &SearchPlacesWidget::centerMap);
    resultsLayout->addWidget(m_results);
    m_results->setEnabled(false);
}

void SearchPlacesWidget::startSearch()
{
    if (m_searchTerm->text().isEmpty()) {
        m_results->clear();
        return;
    }

    QApplication::setOverrideCursor(Qt::BusyCursor);
    setEnabled(false);

    m_results->clear();
    m_results->addItem(i18n("Searching ..."));
    m_searchRunnerManager->findPlacemarks(m_searchTerm->text());
}

void SearchPlacesWidget::showResults(const QList<Marble::GeoDataPlacemark *> &results)
{
    m_results->clear();

    for (const auto &result : results) {
        auto *item = new QListWidgetItem(result->displayName());
        const auto coordinates = result->coordinate();
        item->setData(Qt::UserRole, coordinates.longitude(Marble::GeoDataCoordinates::Degree));
        item->setData(Qt::UserRole + 1, coordinates.latitude(Marble::GeoDataCoordinates::Degree));
        m_results->addItem(item);
    }

    if (results.isEmpty()) {
        m_results->addItem(i18n("No places found"));
        m_results->setEnabled(false);
    } else {
        m_results->setEnabled(true);
    }

    if (results.size() == 1) {
        centerMap(m_results->item(0));
    }

    QApplication::restoreOverrideCursor();
    setEnabled(true);
}

void SearchPlacesWidget::centerMap(QListWidgetItem *item)
{
    m_mapWidget->centerCoordinates(Coordinates(item->data(Qt::UserRole).toDouble(),
                                               item->data(Qt::UserRole + 1).toDouble(),
                                               0, true));
}
