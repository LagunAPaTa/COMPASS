#ifndef EVALUATIONMANAGERMAINTABWIDGET_H
#define EVALUATIONMANAGERMAINTABWIDGET_H

#include <QWidget>

#include <memory>

class EvaluationManager;
class EvaluationManagerWidget;
class EvaluationDataSourceWidget;

class EvaluationManagerMainTabWidget : public QWidget
{
    Q_OBJECT

private slots:
    void dboRefNameChangedSlot(const std::string& dbo_name);
    void dboTstNameChangedSlot(const std::string& dbo_name);

public:
    EvaluationManagerMainTabWidget(EvaluationManager& eval_man, EvaluationManagerWidget& man_widget);

protected:
    EvaluationManager& eval_man_;
    EvaluationManagerWidget& man_widget_;

    std::unique_ptr<EvaluationDataSourceWidget> data_source_ref_widget_ {nullptr};
    std::unique_ptr<EvaluationDataSourceWidget> data_source_tst_widget_ {nullptr};
};

#endif // EVALUATIONMANAGERMAINTABWIDGET_H
