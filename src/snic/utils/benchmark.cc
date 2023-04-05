#include "ns3/benchmark.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("Benchmark");
NS_OBJECT_ENSURE_REGISTERED(Benchmark);

Benchmark::Benchmark(std::string name)
    : m_outputFileNamePrefix(name)
{
}

Benchmark::~Benchmark()
{
}

void
Benchmark::Initialize()
{
    NS_LOG_FUNCTION(this);

    for (uint32_t n = 0; n < m_numExperiments; ++n)
    {
        Experiment e = Experiment(n, m_outputFileNamePrefix);
        e.Initialize(values[n]);
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
Benchmark::AddVariable(std::string varName, std::vector<uint32_t> values)
{
    NS_LOG_FUNCTION(this << varName);
    NS_ASSERT_MSG(values.size() == m_numExperiments, "not enough values for every experiment");
    m_variables[varName] = values;
}

} // namespace ns3
