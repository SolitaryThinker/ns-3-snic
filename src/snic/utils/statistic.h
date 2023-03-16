#ifndef SNIC_STATISTIC_H
#define SNIC_STATISTIC_H

#include "ns3/nstime.h"
#include "ns3/object.h"

namespace ns3
{
    class Statistic : public Object
    {
      public:
        static TypeId GetTypeId();
        Statistic();
        ~Statistic();
        double GetTput() const;
        void AddPacket(uint32_t size);

      private:
        std::string m_label;
        Time m_prev;
        double m_tput;
    };
}

#endif // SNIC_STATISTIC_H
