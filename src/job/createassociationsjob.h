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

#ifndef CREATEASSOCIATIONSJOB_H
#define CREATEASSOCIATIONSJOB_H

#include "job.h"

class CreateAssociationsTask;
class DBInterface;
class Buffer;
class DBObject;

class CreateAssociationsJob : public Job
{
    Q_OBJECT

signals:
    void statusSignal(QString status);

public:
    CreateAssociationsJob(CreateAssociationsTask& task, DBInterface& db_interface,
                          std::map<std::string, std::shared_ptr<Buffer>> buffers);

    virtual ~CreateAssociationsJob();

    virtual void run();

protected:
    CreateAssociationsTask& task_;
    DBInterface& db_interface_;
    std::map<std::string, std::shared_ptr<Buffer>> buffers_;

    std::map<unsigned int, unsigned int> ta_2_utn_;
    unsigned int utn_cnt_ {0};

    void createUTNS();
};

#endif // CREATEASSOCIATIONSJOB_H
