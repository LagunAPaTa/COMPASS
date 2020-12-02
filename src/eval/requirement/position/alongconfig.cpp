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

#include "eval/requirement/position/alongconfig.h"
#include "eval/requirement/position/alongconfigwidget.h"
#include "eval/requirement/group.h"
#include "eval/requirement/base.h"

using namespace std;


namespace EvaluationRequirement
{
    PositionAlongConfig::PositionAlongConfig(
            const std::string& class_id, const std::string& instance_id,
            Group& group, EvaluationStandard& standard, EvaluationManager& eval_man)
    : Config(class_id, instance_id, group, standard, eval_man)
    {
        registerParameter("max_ref_time_diff", &max_ref_time_diff_, 4.0);
        registerParameter("max_abs_value", &max_abs_value_, 50.0);
        registerParameter("minimum_probability", &minimum_probability_, 0.9);
    }

    PositionAlongConfig::~PositionAlongConfig()
    {

    }

    void PositionAlongConfig::addGUIElements(QFormLayout* layout)
    {
        assert (layout);

        Config::addGUIElements(layout);
    }

    PositionAlongConfigWidget* PositionAlongConfig::widget()
    {
        if (!widget_)
            widget_.reset(new PositionAlongConfigWidget(*this));

        return widget_.get();
    }

    std::shared_ptr<Base> PositionAlongConfig::createRequirement()
    {
        shared_ptr<PositionAlong> req = make_shared<PositionAlong>(
                    name_, short_name_, group_.name(), eval_man_,
                    max_ref_time_diff_, max_abs_value_, minimum_probability_);

        return req;
    }

    float PositionAlongConfig::maxAbsValue() const
    {
        return max_abs_value_;
    }

    float PositionAlongConfig::minimumProbability() const
    {
        return minimum_probability_;
    }

    void PositionAlongConfig::maxAbsValue(float value)
    {
        max_abs_value_ = value;
    }

    void PositionAlongConfig::minimumProbability(float value)
    {
        minimum_probability_ = value;
    }

    float PositionAlongConfig::maxRefTimeDiff() const
    {
        return max_ref_time_diff_;
    }

    void PositionAlongConfig::maxRefTimeDiff(float value)
    {
        max_ref_time_diff_ = value;
    }
}
