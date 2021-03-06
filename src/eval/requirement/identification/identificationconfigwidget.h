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

#ifndef EVALUATIONREQUIREMENTIDENTIFICATIONCONFIGWIDGET_H
#define EVALUATIONREQUIREMENTIDENTIFICATIONCONFIGWIDGET_H

#include <QWidget>

class QLineEdit;
class QCheckBox;

class QFormLayout;

namespace EvaluationRequirement
{
    class IdentificationConfig;

    class IdentificationConfigWidget : public QWidget
    {
        Q_OBJECT

    public slots:
        void maxRefTimeDiffEditSlot(QString value);
        void minimumProbEditSlot(QString value);

    public:
        IdentificationConfigWidget(IdentificationConfig& config);

    protected:
        IdentificationConfig& config_;

        QFormLayout* form_layout_ {nullptr};

        QLineEdit* max_ref_time_diff_edit_{nullptr};
        QLineEdit* minimum_prob_edit_{nullptr};
    };

}

#endif // EVALUATIONREQUIREMENTIDENTIFICATIONCONFIGWIDGET_H
