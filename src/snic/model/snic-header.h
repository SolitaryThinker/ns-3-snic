/*
 * Copyright (c) 2023 UCSD WukLab, San Diego, USA
 *
 * Author: Will Lin <wlsaidhi@gmail.com>
 */

#ifndef SNIC_HEADER_H
#define SNIC_HEADER_H

#include "ns3/header.h"

namespace ns3
{

class SnicHeader : public Header
{
  public:
    SnicHeader();

  private:
    bool m_isOffloaded;
    bool m_protocol;
};

} // namespace ns3
#endif // SNIC_HEADER_H
