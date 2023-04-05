#include "ns3/experiment-variable.h"

namespace ns3
{
ExperimentVariable::ExperimentVariable()
{
}

void
ExperimentVariable::SetValues(std::vector<T> values)
{
    m_values = values;
}
} // namespace ns3
