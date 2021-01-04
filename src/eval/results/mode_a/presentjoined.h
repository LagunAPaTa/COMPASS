/*
 * This file is part of OpenATS COMPASS.
 *
 * COMPASS is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * COMPASS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with COMPASS. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef JOINEVALUATIONREQUIREMENTMODEARESULT_H
#define JOINEVALUATIONREQUIREMENTMODEARESULT_H

#include "eval/results/joined.h"

namespace EvaluationRequirementResult
{
    class SingleModeAPresent;

    class JoinedModeAPresent : public Joined
    {
    public:
        JoinedModeAPresent(const std::string& result_id, std::shared_ptr<EvaluationRequirement::Base> requirement,
                             const SectorLayer& sector_layer, EvaluationManager& eval_man);

        virtual void join(std::shared_ptr<Base> other) override;

        //virtual void print() override;
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
        int num_updates_ {0};
        int num_no_ref_pos_ {0};
        int num_no_ref_val_ {0};
        int num_pos_outside_ {0};
        int num_pos_inside_ {0};
        int num_unknown_ {0};
        int num_correct_ {0};
        int num_false_ {0};

        // min exist
        float p_present_min_{0};

        bool has_p_present_ {false};
        float p_present_{0};

        void addToValues (std::shared_ptr<SingleModeAPresent> single_result);
        void updateProbabilities();
        void addToOverviewTable(std::shared_ptr<EvaluationResultsReport::RootItem> root_item);
        void addDetails(std::shared_ptr<EvaluationResultsReport::RootItem> root_item);

        std::unique_ptr<nlohmann::json::object_t> getErrorsViewable ();
    };

}

#endif // JOINEVALUATIONREQUIREMENTMODEARESULT_H
