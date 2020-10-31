#ifndef EVALUATIONSTANDARDTABWIDGET_H
#define EVALUATIONSTANDARDTABWIDGET_H

#include <QWidget>

#include <memory>

class EvaluationManager;
class EvaluationManagerWidget;
class EvaluationStandardComboBox;

class QPushButton;
class QStackedWidget;

class EvaluationStandardTabWidget : public QWidget
{
    Q_OBJECT

private slots:
    void changedStandardsSlot(); // eval man
    void changedCurrentStandardSlot(); // eval man

    void addStandardSlot ();
    void removeStandardSlot ();

public:
    EvaluationStandardTabWidget(EvaluationManager& eval_man, EvaluationManagerWidget& man_widget);

protected:
    EvaluationManager& eval_man_;
    EvaluationManagerWidget& man_widget_;

    std::unique_ptr<EvaluationStandardComboBox> standard_box_ {nullptr};

    QPushButton* add_button_ {nullptr};
    QPushButton* rename_button_ {nullptr};
    QPushButton* copy_button_ {nullptr};
    QPushButton* remove_button_ {nullptr};

    QStackedWidget* standards_widget_{nullptr};

    void updateButtons();
    void updateStandardStack();
};

#endif // EVALUATIONSTANDARDTABWIDGET_H
