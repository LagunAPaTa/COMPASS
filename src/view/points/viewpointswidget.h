#ifndef VIEWPOINTSWIDGET_H
#define VIEWPOINTSWIDGET_H

#include <QWidget>

class ViewManager;
class ViewPointsToolWidget;
class ViewPointsTableModel;

class QTableView;
class QPushButton;
class QSortFilterProxyModel;

class ViewPointsWidget : public QWidget
{
    Q_OBJECT

  public slots:
    void exportSlot();
    void deleteAllSlot();
    void importSlot();

    void currentRowChanged(const QModelIndex& current, const QModelIndex& previous);
    //void onTableClickedSlot(const QModelIndex& current);

    void loadingStartedSlot();
    void allLoadingDoneSlot();

public:
    ViewPointsWidget(ViewManager& view_manager);
    virtual ~ViewPointsWidget();

    void update();

    void selectNext();
    void selectNextOpen();
    void openCurrentSelectNext();
    void closeCurrentSelectNext();

private:
    ViewManager& view_manager_;

    ViewPointsToolWidget* tool_widget_{nullptr};

    QTableView* table_view_{nullptr};
    QSortFilterProxyModel* proxy_model_{nullptr};
    ViewPointsTableModel* table_model_{nullptr};

    QPushButton* import_button_{nullptr};
    QPushButton* delete_all_button_{nullptr};
    QPushButton* export_button_{nullptr};
};

#endif // VIEWPOINTSWIDGET_H