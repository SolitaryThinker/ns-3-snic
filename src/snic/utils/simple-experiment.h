#ifndef EXPERIMENT_H
#define EXPERIMENT_H

#include "ns3/experiment.h"
#include "ns3/object.h"

namespace ns3
{
NS_LOG_COMPONENT_DEFINE("SimpleExperiment");
NS_OBJECT_ENSURE_REGISTERED(SimpleExperiment);

class SimpleExperiment : public Experiment
{
  public:
    SimpleExperiment();
    ~SimpleExperiment();

    virtual void Initialize();
    virtual void Run() override;

  private:
    std::string m_outputFileName;
};

} // namespace ns3
#endif // EXPERIMENT_H
