#ifndef EVALUATIONREQUIREMENPOSITIONMAXDISTANCERESULT_H
#define EVALUATIONREQUIREMENPOSITIONMAXDISTANCERESULT_H

#include "eval/results/single.h"
#include "eval/requirement/position/positionmaxdistance.h"

namespace EvaluationRequirementResult
{

class SinglePositionMaxDistance : public Single
{
public:
    SinglePositionMaxDistance(
            const std::string& result_id, std::shared_ptr<EvaluationRequirement::Base> requirement,
            unsigned int utn, const EvaluationTargetData* target, EvaluationManager& eval_man,
            int num_pos, int num_no_ref, int num_pos_ok, int num_pos_nok);

    virtual void print() override;
    virtual void addToReport (std::shared_ptr<EvaluationResultsReport::RootItem> root_item) override;

    virtual std::shared_ptr<Joined> createEmptyJoined(const std::string& result_id) override;

    int numPos() const;
    int numNoRef() const;
    int numPosOk() const;
    int numPosNOk() const;

protected:
    int num_pos_ {0};
    int num_no_ref_ {0};
    int num_pos_ok_ {0};
    int num_pos_nok_ {0};

    bool has_p_max_pos_ {false};
    float p_max_pos_{0};

    void updatePMaxPos();
};

}

#endif // EVALUATIONREQUIREMENPOSITIONMAXDISTANCERESULT_H
