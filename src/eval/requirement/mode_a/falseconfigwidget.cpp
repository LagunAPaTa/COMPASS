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

#include "eval/requirement/mode_a/falseconfigwidget.h"
#include "eval/requirement/mode_a/falseconfig.h"
#include "textfielddoublevalidator.h"
#include "logger.h"

#include <QLineEdit>
#include <QFormLayout>
#include <QCheckBox>

using namespace std;

namespace EvaluationRequirement
{

    ModeAFalseConfigWidget::ModeAFalseConfigWidget(ModeAFalseConfig& config)
            : QWidget(), config_(config)
    {
        form_layout_ = new QFormLayout();

        config_.addGUIElements(form_layout_);

        // max ref time diff
        max_ref_time_diff_edit_ = new QLineEdit(QString::number(config_.maxRefTimeDiff()));
        max_ref_time_diff_edit_->setValidator(new QDoubleValidator(0.0, 30.0, 2, this));
        connect(max_ref_time_diff_edit_, &QLineEdit::textEdited,
                this, &ModeAFalseConfigWidget::maxRefTimeDiffEditSlot);

        form_layout_->addRow("Maximum Reference Time Difference [s]", max_ref_time_diff_edit_);

        // false prob
        max_prob_false_edit_ = new QLineEdit(QString::number(config_.maximumProbabilityFalse()));
        max_prob_false_edit_->setValidator(new QDoubleValidator(0.0001, 1.0, 4, this));
        connect(max_prob_false_edit_, &QLineEdit::textEdited,
                this, &ModeAFalseConfigWidget::maxProbFalseEditSlot);

        form_layout_->addRow("False Maximum Probability [1]", max_prob_false_edit_);

        setLayout(form_layout_);
    }

    void ModeAFalseConfigWidget::maxRefTimeDiffEditSlot(QString value)
    {
        loginf << "ModeAConfigWidget: maxRefTimeDiffEditSlot: value " << value.toStdString();

        bool ok;
        float val = value.toFloat(&ok);

        if (ok)
            config_.maxRefTimeDiff(val);
        else
            loginf << "ModeAConfigWidget: maxRefTimeDiffEditSlot: invalid value";
    }

    void ModeAFalseConfigWidget::maxProbFalseEditSlot(QString value)
    {
        loginf << "EvaluationRequirementModeAConfigWidget: maxProbFalseEditSlot: value " << value.toStdString();

        bool ok;
        float val = value.toFloat(&ok);

        if (ok)
            config_.maximumProbabilityFalse(val);
        else
            loginf << "EvaluationRequirementModeAConfigWidget: maxProbFalseEditSlot: invalid value";
    }

}
