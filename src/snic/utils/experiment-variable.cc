#include "ns3/experiment-variable.h"

namespace ns3
{

ExperimentVariable::ExperimentVariable(std::string varName, Ptr<const AttributeChecker> checker)
    : m_varName(varName),
      m_checker(checker)
{
}

void
ExperimentVariable::SetValues(std::vector<Ptr<AttributeValue>> values)
{
    m_values = values;
}

std::vector<Ptr<AttributeValue>>
ExperimentVariable::GetValues() const
{
    return m_values;
}

Ptr<AttributeValue>
ExperimentVariable::GetValue(uint64_t n) const
{
    return m_values[n];
}

} // namespace ns3
