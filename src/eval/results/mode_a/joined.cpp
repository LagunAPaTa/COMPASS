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

#include "eval/results/mode_a/single.h"
#include "eval/results/mode_a/joined.h"
#include "eval/requirement/base.h"
#include "eval/requirement/mode_a/modea.h"
#include "evaluationtargetdata.h"
#include "evaluationmanager.h"
#include "eval/results/report/rootitem.h"
#include "eval/results/report/section.h"
#include "eval/results/report/sectioncontenttext.h"
#include "eval/results/report/sectioncontenttable.h"
#include "logger.h"
#include "stringconv.h"

#include <cassert>

using namespace std;
using namespace Utils;

namespace EvaluationRequirementResult
{

    JoinedModeA::JoinedModeA(
            const std::string& result_id, std::shared_ptr<EvaluationRequirement::Base> requirement,
            const SectorLayer& sector_layer, EvaluationManager& eval_man)
        : Joined("JoinedModeA", result_id, requirement, sector_layer, eval_man)
    {
        std::shared_ptr<EvaluationRequirement::ModeA> req =
                std::static_pointer_cast<EvaluationRequirement::ModeA>(requirement_);
        assert (req);

        use_p_present_req_ = req->useMinimumProbabilityPresent();
        p_present_min_ = req->minimumProbabilityPresent();

        use_p_false_req_ = req->useMaximumProbabilityFalse();
        p_false_max_ = req->maximumProbabilityFalse();
    }


    void JoinedModeA::join(std::shared_ptr<Base> other)
    {
        Joined::join(other);

        std::shared_ptr<SingleModeA> other_sub =
                std::static_pointer_cast<SingleModeA>(other);
        assert (other_sub);

        addToValues(other_sub);
    }

    void JoinedModeA::addToValues (std::shared_ptr<SingleModeA> single_result)
    {
        assert (single_result);

        if (!single_result->use())
            return;

        num_updates_ += single_result->numUpdates();
        num_no_ref_pos_ += single_result->numNoRefPos();
        num_no_ref_val_ += single_result->numNoRefValue();
        num_pos_outside_ += single_result->numPosOutside();
        num_pos_inside_ += single_result->numPosInside();
        num_unknown_ += single_result->numUnknown();
        num_correct_ += single_result->numCorrect();
        num_false_ += single_result->numFalse();

        updateProbabilities();
    }

    void JoinedModeA::updateProbabilities()
    {
        assert (num_updates_ - num_no_ref_pos_ == num_pos_inside_ + num_pos_outside_);
        assert (num_pos_inside_ == num_no_ref_val_+num_unknown_+num_correct_+num_false_);

        if (num_correct_+num_false_)
        {
            p_present_ = (float)(num_correct_+num_false_)/(float)(num_correct_+num_false_+num_unknown_);
            has_p_present_ = true;

            p_false_ = (float)(num_false_)/(float)(num_correct_+num_false_);
            has_p_false_ = true;
        }
        else
        {
            has_p_present_ = false;
            p_present_ = 0;

            has_p_false_ = false;
            p_false_ = 0;
        }
    }

    void JoinedModeA::addToReport (
            std::shared_ptr<EvaluationResultsReport::RootItem> root_item)
    {
        logdbg << "JoinedModeA " <<  requirement_->name() <<": addToReport";

        if (!results_.size()) // some data must exist
        {
            logerr << "JoinedModeA " <<  requirement_->name() <<": addToReport: no data";
            return;
        }

        logdbg << "JoinedModeA " <<  requirement_->name() << ": addToReport: adding joined result";

        addToOverviewTable(root_item);
        addDetails(root_item);
    }

    void JoinedModeA::addToOverviewTable(std::shared_ptr<EvaluationResultsReport::RootItem> root_item)
    {
        EvaluationResultsReport::SectionContentTable& ov_table = getReqOverviewTable(root_item);

        // condition
        std::shared_ptr<EvaluationRequirement::ModeA> req =
                std::static_pointer_cast<EvaluationRequirement::ModeA>(requirement_);
        assert (req);

        // p present
        {
            QVariant pe_var;

            string condition = ">= "+String::percentToString(p_present_min_ * 100.0);

            string result {"Unknown"};

            if (has_p_present_)
            {
                result = p_present_ >= p_present_min_ ? "Passed" : "Failed";
                pe_var = roundf(p_present_ * 10000.0) / 100.0;
            }

            // "Sector Layer", "Group", "Req.", "Id", "#Updates", "Result", "Condition", "Result"
            ov_table.addRow({sector_layer_.name().c_str(), requirement_->groupName().c_str(),
                             (requirement_->shortname()+" Present").c_str(),
                             result_id_.c_str(), {num_correct_+num_false_},
                             pe_var, condition.c_str(), result.c_str()}, this, {});
        }

        // p false
        {
            QVariant pf_var;

            string condition = "<= "+String::percentToString(p_false_max_ * 100.0);

            string result {"Unknown"};

            if (has_p_false_)
            {
                result = p_false_ <= p_false_max_ ? "Passed" : "Failed";
                pf_var = roundf(p_false_ * 10000.0) / 100.0;
            }

            // "Sector Layer", "Group", "Req.", "Id", "#Updates", "Result", "Condition", "Result"
            ov_table.addRow({sector_layer_.name().c_str(), requirement_->groupName().c_str(),
                             (requirement_->shortname()+" False").c_str(),
                             result_id_.c_str(), {num_correct_+num_false_},
                             pf_var, condition.c_str(), result.c_str()}, this, {});
        }


    }

    void JoinedModeA::addDetails(std::shared_ptr<EvaluationResultsReport::RootItem> root_item)
    {
        EvaluationResultsReport::Section& sector_section = getRequirementSection(root_item);

        if (!sector_section.hasTable("sector_details_table"))
            sector_section.addTable("sector_details_table", 3, {"Name", "comment", "Value"}, false);

        EvaluationResultsReport::SectionContentTable& sec_det_table =
                sector_section.getTable("sector_details_table");

        addCommonDetails(sec_det_table);

        sec_det_table.addRow({"Use", "To be used in results", use_}, this);
        sec_det_table.addRow({"#Up [1]", "Number of updates", num_updates_}, this);
        sec_det_table.addRow({"#NoRef [1]", "Number of updates w/o reference position or code",
                              num_no_ref_pos_+num_no_ref_val_}, this);
        sec_det_table.addRow({"#NoRefPos [1]", "Number of updates w/o reference position ", num_no_ref_pos_}, this);
        sec_det_table.addRow({"#NoRef [1]", "Number of updates w/o reference code", num_no_ref_val_}, this);
        sec_det_table.addRow({"#PosInside [1]", "Number of updates inside sector", num_pos_inside_}, this);
        sec_det_table.addRow({"#PosOutside [1]", "Number of updates outside sector", num_pos_outside_}, this);
        sec_det_table.addRow({"#Unknown [1]", "Number of updates unknown code", num_unknown_}, this);
        sec_det_table.addRow({"#Correct [1]", "Number of updates with correct code", num_correct_}, this);
        sec_det_table.addRow({"#False [1]", "Number of updates with false code", num_false_}, this);

        // condition
        {
            QVariant pe_var;

            if (has_p_present_)
                pe_var = roundf(p_present_ * 10000.0) / 100.0;

            sec_det_table.addRow({"PP [%]", "Probability of Mode 3/A present", pe_var}, this);

            string condition = ">= "+String::percentToString(p_present_min_ * 100.0);

            sec_det_table.addRow(
            {"Condition Present", ("Use: "+to_string(use_p_present_req_)).c_str(), condition.c_str()}, this);

            string result {"Unknown"};

            if (has_p_present_)
                result = p_present_ >= p_present_min_ ? "Passed" : "Failed";

            sec_det_table.addRow({"Condition Present Fulfilled", "", result.c_str()}, this);
        }

        {
            QVariant pf_var;

            if (has_p_false_)
                pf_var = roundf(p_false_ * 10000.0) / 100.0;

            sec_det_table.addRow({"PF [%]", "Probability of Mode 3/A false", pf_var}, this);

            string condition = "<= "+String::percentToString(p_false_max_ * 100.0);

            sec_det_table.addRow(
            {"Condition False", ("Use: "+to_string(use_p_false_req_)).c_str(), condition.c_str()}, this);

            string result {"Unknown"};

            if (has_p_false_)
                result = p_false_ <= p_false_max_ ? "Passed" : "Failed";

            sec_det_table.addRow({"Condition False Fulfilled", "", result.c_str()}, this);
        }

        // figure
        if ((has_p_present_ && p_present_ != 1.0)
                || (has_p_false_ && p_false_ != 0.0))
        {
            sector_section.addFigure("sector_errors_overview", "Sector Errors Overview",
                                     getErrorsViewable());
        }
        else
        {
            sector_section.addText("sector_errors_overview_no_figure");
            sector_section.getText("sector_errors_overview_no_figure").addText(
                        "No target errors found, therefore no figure was generated.");
        }
    }


    bool JoinedModeA::hasViewableData (
            const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation)
    {
        //loginf << "UGA4 '"  << table.name() << "'" << " other '" << req_overview_table_name_ << "'";

        if (table.name() == req_overview_table_name_)
            return true;
        else
            return false;
    }

    std::unique_ptr<nlohmann::json::object_t> JoinedModeA::viewableData(
            const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation)
    {
        assert (hasViewableData(table, annotation));

        return getErrorsViewable();
    }

    std::unique_ptr<nlohmann::json::object_t> JoinedModeA::getErrorsViewable ()
    {
        std::unique_ptr<nlohmann::json::object_t> viewable_ptr =
                eval_man_.getViewableForEvaluation(req_grp_id_, result_id_);

        double lat_min, lat_max, lon_min, lon_max;

        tie(lat_min, lat_max) = sector_layer_.getMinMaxLatitude();
        tie(lon_min, lon_max) = sector_layer_.getMinMaxLongitude();

        (*viewable_ptr)["position_latitude"] = (lat_max+lat_min)/2.0;
        (*viewable_ptr)["position_longitude"] = (lon_max+lon_min)/2.0;;

        double lat_w = 1.1*(lat_max-lat_min)/2.0;
        double lon_w = 1.1*(lon_max-lon_min)/2.0;

        if (lat_w < 0.02)
            lat_w = 0.02;

        if (lon_w < 0.02)
            lon_w = 0.02;

        (*viewable_ptr)["position_window_latitude"] = lat_w;
        (*viewable_ptr)["position_window_longitude"] = lon_w;

        return viewable_ptr;
    }

    bool JoinedModeA::hasReference (
            const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation)
    {
        //loginf << "UGA5 '"  << table.name() << "'" << " other '" << req_overview_table_name_ << "'";

        if (table.name() == req_overview_table_name_)
            return true;
        else
            return false;;
    }

    std::string JoinedModeA::reference(
            const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation)
    {
        assert (hasReference(table, annotation));
        return "Report:Results:"+getRequirementSectionID();

        return nullptr;
    }

    void JoinedModeA::updatesToUseChanges()
    {
        loginf << "JoinedModeA: updatesToUseChanges: prev num_updates " << num_updates_
               << " num_no_ref_pos " << num_no_ref_pos_ << " num_no_ref_id " << num_no_ref_val_
               << " num_unknown_id " << num_unknown_
               << " num_correct_id " << num_correct_ << " num_false_id " << num_false_;

//        if (has_pid_)
//            loginf << "JoinedModeA: updatesToUseChanges: prev result " << result_id_
//                   << " pid " << 100.0 * pid_;
//        else
//            loginf << "JoinedModeA: updatesToUseChanges: prev result " << result_id_ << " has no data";

        num_updates_ = 0;
        num_no_ref_pos_ = 0;
        num_no_ref_val_ = 0;
        num_pos_outside_ = 0;
        num_pos_inside_ = 0;
        num_unknown_ = 0;
        num_correct_ = 0;
        num_false_ = 0;

        for (auto result_it : results_)
        {
            std::shared_ptr<SingleModeA> result =
                    std::static_pointer_cast<SingleModeA>(result_it);
            assert (result);

            addToValues(result);
        }

        loginf << "JoinedModeA: updatesToUseChanges: updt num_updates " << num_updates_
               << " num_no_ref_pos " << num_no_ref_pos_ << " num_no_ref_id " << num_no_ref_val_
               << " num_unknown_id " << num_unknown_
               << " num_correct_id " << num_correct_ << " num_false_id " << num_false_;

//        if (has_pid_)
//            loginf << "JoinedModeA: updatesToUseChanges: updt result " << result_id_
//                   << " pid " << 100.0 * pid_;
//        else
//            loginf << "JoinedModeA: updatesToUseChanges: updt result " << result_id_ << " has no data";
    }

}
