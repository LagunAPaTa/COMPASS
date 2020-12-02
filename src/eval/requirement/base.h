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

#ifndef EVALUATIONREQUIREMENT_H
#define EVALUATIONREQUIREMENT_H

#include <string>
#include <memory>

class EvaluationTargetData;
class EvaluationManager;
class SectorLayer;

namespace EvaluationRequirementResult {
    class Single;
}

namespace EvaluationRequirement
{

class Base
{
public:
    Base(const std::string& name, const std::string& short_name, const std::string& group_name,
                          EvaluationManager& eval_man);

    virtual std::shared_ptr<EvaluationRequirementResult::Single> evaluate (
            const EvaluationTargetData& target_data, std::shared_ptr<Base> instance,
            const SectorLayer& sector_layer) = 0;
    // instance is the self-reference for the result

    std::string name() const;
    std::string shortname() const;
    std::string groupName() const;

protected:
    static bool in_appimage_;

    std::string name_;
    std::string short_name_;
    std::string group_name_;

    EvaluationManager& eval_man_;
};

}

#endif // EVALUATIONREQUIREMENT_H
