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

#include "nullablevector.h"

template <>
NullableVector<bool>& NullableVector<bool>::operator*=(double factor)
{
    bool tmp_factor = static_cast<bool>(factor);

    //    for (auto data_it : data_)
    //        data_it = data_it && tmp_factor;

    unsigned int data_size = data_.size();

    tbb::parallel_for(uint(0), data_size, [&](unsigned int cnt) {
        if (!isNull(cnt))
        {
            data_.at(cnt) = data_.at(cnt) && tmp_factor;
        }
    });

    return *this;
}

template <>
void NullableVector<bool>::append(unsigned int index, bool value)
{
    logdbg << "ArrayListTemplate " << property_.name() << ": append: index " << index << " value '"
           << value << "'";

    if (BUFFER_PEDANTIC_CHECKING)
    {
        assert(data_.size() <= buffer_.data_size_);
        assert(null_flags_.size() <= buffer_.data_size_);
    }

    if (index >= data_.size())  // allocate new stuff, fill all new with not null
    {
        if (index != data_.size())  // some where left out
            resizeNullTo(index + 1);

        resizeDataTo(index + 1);
    }

    if (BUFFER_PEDANTIC_CHECKING)
        assert(index < data_.size());

    data_.at(index) = data_.at(index) || value;

    unsetNull(index);

    // logdbg << "ArrayListTemplate: append: size " << size_ << " max_size " << max_size_;
}

template <>
void NullableVector<std::string>::append(unsigned int index, std::string value)
{
    logdbg << "ArrayListTemplate " << property_.name() << ": append: index " << index << " value '"
           << value << "'";

    if (BUFFER_PEDANTIC_CHECKING)
    {
        assert(data_.size() <= buffer_.data_size_);
        assert(null_flags_.size() <= buffer_.data_size_);
    }

    if (index >= data_.size())  // allocate new stuff, fill all new with not null
    {
        if (index != data_.size())  // some where left out
            resizeNullTo(index + 1);

        resizeDataTo(index + 1);
    }

    if (BUFFER_PEDANTIC_CHECKING)
        assert(index < data_.size());

    if (data_.at(index).size())
        data_.at(index) += ";" + value;
    else
        data_.at(index) = value;

    unsetNull(index);

    // logdbg << "ArrayListTemplate: append: size " << size_ << " max_size " << max_size_;
}
