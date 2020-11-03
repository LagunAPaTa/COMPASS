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

#ifndef CONFIGURABLPPARAMETER_H
#define CONFIGURABLPPARAMETER_H

#include <string>

#include "json.hpp"

class Configurable;

/**
 * @brief Configuration parameter template class
 *
 * @details Several types of parameters are implement here:
 * "ParameterBool", "ParameterInt", "ParameterUnsignedInt", "ParameterFloat", "ParameterDouble",
 * "ParameterString"
 *
 * For all such types one template class was implemented to unify setting and retrieving the XML
 * configuration.
 */
template <class T>
class ConfigurableParameter
{
  public:
    /// Constructor, initializes members
    explicit ConfigurableParameter() {}

    /// Copy constructor, uses assignment operator
    ConfigurableParameter(const ConfigurableParameter& source);

    /// Assignment operator, make deep copy and discards the pointer.
    virtual ConfigurableParameter& operator=(const ConfigurableParameter& source);

    /// Destructor
    virtual ~ConfigurableParameter() {}

    /// Parameter identifier
    std::string parameter_id_;
    /// Template pointer to real value
    T* pointer_{nullptr};
    /// Template configuration value
    T config_value_;
    /// Template default value as given by registerParameter
    T default_value_;

    /**
     * Returns the parameter type as string
     *
     * \exception std::runtime_error if unknown class is used
     */
    std::string getParameterType() const;

    /// Returns parameter identifier
    const std::string& getParameterId() const;

    /// Returns parameter value as string (using a stringstream)
    std::string getParameterValueString() const;
    T getParameterValue() const;

    /// Sets pointer_ to default value if valid, otherwise sets config_value_ to default_value_
    void resetToDefault();
};

// template<> void A<int>::AFnc(); // <- note, no function body

template <>
std::string ConfigurableParameter<bool>::getParameterType() const;
template <>
std::string ConfigurableParameter<int>::getParameterType() const;
template <>
std::string ConfigurableParameter<unsigned int>::getParameterType() const;
template <>
std::string ConfigurableParameter<float>::getParameterType() const;
template <>
std::string ConfigurableParameter<double>::getParameterType() const;
template <>
std::string ConfigurableParameter<std::string>::getParameterType() const;
template <>
std::string ConfigurableParameter<nlohmann::json>::getParameterType() const;

template <>
std::string ConfigurableParameter<bool>::getParameterValueString() const;
template <>
std::string ConfigurableParameter<int>::getParameterValueString() const;
template <>
std::string ConfigurableParameter<unsigned int>::getParameterValueString() const;
template <>
std::string ConfigurableParameter<float>::getParameterValueString() const;
template <>
std::string ConfigurableParameter<double>::getParameterValueString() const;
template <>
std::string ConfigurableParameter<std::string>::getParameterValueString() const;
template <>
std::string ConfigurableParameter<nlohmann::json>::getParameterValueString() const;

#endif  // CONFIGURABLPPARAMETER_H
