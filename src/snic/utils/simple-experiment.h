#ifndef SIMPLE_EXPERIMENT_H
#define SIMPLE_EXPERIMENT_H

#include "ns3/experiment.h"
#include "ns3/object.h"
#include "ns3/simulator.h"

namespace ns3
{

class SimpleExperiment : public Experiment
{
  public:
    SimpleExperiment(uint32_t id, std::string prefix);
    ~SimpleExperiment();

    virtual void Initialize(std::map<std::string, std::vector<Ptr<AttributeValue>>> variables,
                            std::map<std::string, uint32_t> indexes) override;
    virtual void Run() override;

  private:
    std::string m_outputFileName;
};

} // namespace ns3
#endif // SIMPLE_EXPERIMENT_H
