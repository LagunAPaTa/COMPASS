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

#ifndef EVALUATIONREQUIREMENTDETECTIONCONFIGWIDGET_H
#define EVALUATIONREQUIREMENTDETECTIONCONFIGWIDGET_H

#include <QWidget>

class QLineEdit;
class QCheckBox;

class QFormLayout;

namespace EvaluationRequirement
{
    class DetectionConfig;

    class DetectionConfigWidget : public QWidget
    {
        Q_OBJECT

    public slots:
        void updateIntervalEditSlot(QString value);
        void maxRefTimeDiffEditSlot(QString value);
        void minimumProbEditSlot(QString value);

//        void toggleUseMaxGapSlot();
//        void maxGapEditSlot(QString value);

        void toggleUseMissToleranceSlot();
        void missToleranceEditSlot(QString value);

    public:
        DetectionConfigWidget(DetectionConfig& config);

    protected:
        DetectionConfig& config_;

        QFormLayout* form_layout_ {nullptr};

        QLineEdit* update_interval_edit_{nullptr};
        QLineEdit* max_ref_time_diff_edit_{nullptr};
        QLineEdit* minimum_prob_edit_{nullptr};

        //QCheckBox* use_max_gap_check_{nullptr};
        //QLineEdit* max_gap_interval_edit_{nullptr};

        QCheckBox* use_miss_tolerance_check_{nullptr};
        QLineEdit* miss_tolerance_edit_{nullptr};

    };

}

#endif // EVALUATIONREQUIREMENTDETECTIONCONFIGWIDGET_H
