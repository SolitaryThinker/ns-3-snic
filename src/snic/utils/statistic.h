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
        double m_total_tput;
        uint64_t m_count;
        bool m_first;
    };
}

#endif // SNIC_STATISTIC_H
