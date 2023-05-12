#ifndef BENCHMARK_H
#define BENCHMARK_H

#include "ns3/experiment-variable.h"
#include "ns3/experiment.h"
#include "ns3/log.h"
#include "ns3/object.h"

namespace ns3
{
class Benchmark : public Object
{
  public:
    Benchmark(std::string name);
    ~Benchmark();

    void Initialize();
    void Run();

    // virtual void AddVariable(std::string varName, std::vector<Ptr<AttributeValue>> values);
    virtual void AddVariable(ExperimentVariable var);

  private:
    std::string m_outputFileNamePrefix;
    std::list<Experiment> m_experiments;
    // std::map<std::string, std::vector<Ptr<AttributeValue>>> m_variables;
    std::vector<ExperimentVariable> m_variables;
    std::map<std::string, bool> m_experimentsAdded;
    uint32_t m_numExperiments;
};
} // namespace ns3
#endif // BENCHMARK_H
