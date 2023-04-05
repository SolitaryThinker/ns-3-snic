#ifndef EXPERIMENT_VARIABLE_H
#define EXPERIMENT_VARIABLE_H

#include "ns3/object.h"

namespace ns3
{
template <typename T>
class ExperimentVariable : public Object
{
  public:
    ExperimentVariable()
    {
    }

    void SetValues(std::vector<T> values)
    {
        m_values = values;
    }

    std::vector<T> GetValues() const
    {
        return m_values;
    }

    T GetValue(uint64_t n) const
    {
        return m_values[n];
    }

  private:
    std::vector<T> m_values;
};

class ExperimentVariableU32 : public ExperimentVariable<uint32_t>
{
  public:
    ExperimentVariableU32();
};

} // namespace ns3
#endif // EXPERIMENT_VARIABLE_H
