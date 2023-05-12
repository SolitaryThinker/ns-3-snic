#ifndef EXPERIMENT_VARIABLE_H
#define EXPERIMENT_VARIABLE_H

#include "ns3/object.h"

namespace ns3
{
class ExperimentVariable : public Object
{
  public:
    ExperimentVariable(std::string varName, Ptr<const AttributeChecker> checker);

    void SetValues(std::vector<Ptr<AttributeValue>> values);

    std::vector<Ptr<AttributeValue>> GetValues() const;

    Ptr<AttributeValue> GetValue(uint64_t n) const;

  private:
    std::string m_varName;
    Ptr<const AttributeChecker> m_checker;
    std::vector<Ptr<AttributeValue>> m_values;
};

} // namespace ns3
#endif // EXPERIMENT_VARIABLE_H
