#ifndef SINGLEEVALUATIONREQUIREMENTDETECTIONRESULT_H
#define SINGLEEVALUATIONREQUIREMENTDETECTIONRESULT_H

#include "eval/results/base.h"

namespace EvaluationRequirementResult
{
    using namespace std;

    class Joined;

    class Single : public Base
    {
    public:
        Single(const std::string& type, const std::string& result_id,
               std::shared_ptr<EvaluationRequirement::Base> requirement, const SectorLayer& sector_layer,
               unsigned int utn, const EvaluationTargetData* target, EvaluationManager& eval_man);

        virtual bool isSingle() const override { return true; }
        virtual bool isJoined() const override { return false; }

        virtual void print() = 0;
        virtual void addToReport (std::shared_ptr<EvaluationResultsReport::RootItem> root_item) = 0;

        virtual std::shared_ptr<Joined> createEmptyJoined(const std::string& result_id) = 0;

        unsigned int utn() const;
        const EvaluationTargetData* target() const;

        void updateUseFromTarget ();

    protected:
        unsigned int utn_; // used to generate result
        const EvaluationTargetData* target_; // used to generate result

        bool result_usable_ {true}; // whether valid data exists, changed in subclass

        std::string getTargetSectionID ();
    };

}

#endif // SINGLEEVALUATIONREQUIREMENTDETECTIONRESULT_H
