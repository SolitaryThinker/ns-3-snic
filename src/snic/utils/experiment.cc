#include "experiment.h"

namespace ns3
{
NS_LOG_COMPONENT_DEFINE("Experiment");
NS_OBJECT_ENSURE_REGISTERED(Experiment);

Experiment::Experiment(uint32_t id, std::string prefix)
    : m_id(id),
      m_outputFileName(std::to_string(id) + prefix)
{
}

Experiment::~Experiment()
{
}

void
Experiment::Initialize(std::map<std::string, std::vector<Ptr<AttributeValue>>> variables,
                       std::map<std::string, uint32_t> indexes)
{
    NS_LOG_FUNCTION(this);
}

void
Experiment::Run()
{
}
} // namespace ns3
