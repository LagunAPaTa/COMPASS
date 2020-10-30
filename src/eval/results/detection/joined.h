#ifndef JOINEDEVALUATIONREQUIREMENTDETECTIONRESULT_H
#define JOINEDEVALUATIONREQUIREMENTDETECTIONRESULT_H

#include "eval/results/joined.h"

namespace EvaluationRequirementResult
{
    class SingleDetection;

    class JoinedDetection : public Joined
    {
    public:
        JoinedDetection(const std::string& result_id, std::shared_ptr<EvaluationRequirement::Base> requirement,
                        const SectorLayer& sector_layer, EvaluationManager& eval_man);

        virtual void join(std::shared_ptr<Base> other) override;

        virtual void print() override;
        virtual void addToReport (std::shared_ptr<EvaluationResultsReport::RootItem> root_item) override;

        virtual void updatesToUseChanges() override;

        virtual bool hasViewableData (
                const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation) override;
        virtual std::unique_ptr<nlohmann::json::object_t> viewableData(
                const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation) override;

        virtual bool hasReference (
                const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation) override;
        virtual std::string reference(
                const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation) override;

    protected:
        int sum_uis_ {0};
        int missed_uis_ {0};

        bool has_pd_ {false};
        float pd_{0};

        void addToValues (std::shared_ptr<SingleDetection> single_result);
        void updatePD();

        void addToOverviewTable(std::shared_ptr<EvaluationResultsReport::RootItem> root_item);
        void addDetails(std::shared_ptr<EvaluationResultsReport::RootItem> root_item);

        std::unique_ptr<nlohmann::json::object_t> getErrorsViewable ();
    };

}

#endif // JOINEDEVALUATIONREQUIREMENTDETECTIONRESULT_H
