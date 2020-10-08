#ifndef EVALUATIONSTANDARD_H
#define EVALUATIONSTANDARD_H

#include <QObject>
#include <QMenu>

#include <memory>

#include "configurable.h"
#include "evaluationstandardtreeitem.h"
#include "evaluationstandardrootitem.h"

class EvaluationManager;
class Group;
class EvaluationStandardWidget;
class EvaluationStandardTreeModel;

class EvaluationStandard : public QObject, public Configurable, public EvaluationStandardTreeItem
{
    Q_OBJECT

signals:
    void groupsChangedSignal();

public slots:
    void addGroupSlot();

public:
    EvaluationStandard(const std::string& class_id, const std::string& instance_id, EvaluationManager& eval_man);
    virtual ~EvaluationStandard();

    virtual void generateSubConfigurable(const std::string& class_id,
                                         const std::string& instance_id) override;

    std::string name() const;

    bool hasGroup (const std::string& name);
    void addGroup (const std::string& name);
    Group& group (const std::string& name);
    void removeGroup (const std::string& name);

    using EvaluationRequirementGroupIterator =
    typename std::map<std::string, std::unique_ptr<Group>>::iterator;

    EvaluationRequirementGroupIterator begin() { return groups_.begin(); }
    EvaluationRequirementGroupIterator end() { return groups_.end(); }
    unsigned int size () { return groups_.size(); };

    EvaluationStandardWidget* widget();

    virtual EvaluationStandardTreeItem *child(int row) override;
    virtual int childCount() const override;
    virtual int columnCount() const override;
    virtual QVariant data(int column) const override;
    virtual int row() const override;

    EvaluationStandardRootItem& rootItem();

    void showMenu ();
    void beginModelReset();
    void endModelReset();

protected:
    EvaluationManager& eval_man_;
    std::string name_;

    EvaluationStandardRootItem root_item_;

    std::unique_ptr<EvaluationStandardWidget> widget_;

    std::map<std::string, std::unique_ptr<Group>> groups_;

    QMenu menu_;

    virtual void checkSubConfigurables() override;
};

#endif // EVALUATIONSTANDARD_H
