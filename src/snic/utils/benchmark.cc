#include "ns3/benchmark.h"

#include "ns3/simple-experiment.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("Benchmark");
NS_OBJECT_ENSURE_REGISTERED(Benchmark);

Benchmark::Benchmark(std::string name)
    : m_outputFileNamePrefix(name),
      m_numExperiments(0)
{
}

Benchmark::~Benchmark()
{
}

void
Benchmark::Initialize()
{
    NS_LOG_FUNCTION(this);
    NS_LOG_DEBUG("adding " << m_numExperiments << " experiments");

    std::map<std::string, uint32_t> indexes;


    for (uint32_t n = 0; n < m_numExperiments; ++n)
    {
        Experiment e = SimpleExperiment(n, m_outputFileNamePrefix);
        // XXX hack
        for (const auto& kv : m_variables)
        {
            indexes[kv.first] = n;
        }

        e.Initialize(m_variables, indexes);
        m_experiments.push_back(e);
    }
}

void
Benchmark::Run()
{
    NS_LOG_FUNCTION(this);

    for (Experiment& experiment : m_experiments)
    {
        experiment.Run();
    }
}

void
Benchmark::AddVariable(std::string varName, std::vector<Ptr<AttributeValue>> values)
{
    NS_LOG_FUNCTION(this << varName);
    NS_ASSERT_MSG(values.size() == m_numExperiments, "not enough values for every experiment");
    m_variables[varName] = values;
    if (m_numExperiments == 0)
    {
        m_numExperiments = values.size();
    }
    else
    {
        m_numExperiments *= values.size();
    }
}

} // namespace ns3
