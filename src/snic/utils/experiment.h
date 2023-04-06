#ifndef EXPERIMENT_H
#define EXPERIMENT_H

#include "ns3/log.h"
#include "ns3/object.h"

namespace ns3
{

// An experiment is a single run with a single setting for a range of values
class Experiment : public Object
{
  public:
    Experiment(uint32_t id, std::string prefix);
    ~Experiment();

    virtual void Initialize(std::map<std::string, std::vector<Ptr<AttributeValue>>> variables,
                            std::map<std::string, uint32_t> indexes);
    virtual void Run();

    uint16_t GetPacketSize(uint64_t idx);
    uint16_t GetInterval(uint64_t idx);
    uint16_t GetFlowSize(uint64_t idx);
    uint16_t FlowPktCount(uint64_t idx);

    std::string GetName() const;

  protected:
    bool m_initialized;
    std::string m_outputFileName;

  private:
    uint32_t m_id;
};

} // namespace ns3
#endif // EXPERIMENT_H
