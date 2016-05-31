/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil -*- */

// custom-strategy.h
        
#ifndef CUSTOM_STRATEGY_H
#define CUSTOM_STRATEGY_H

#include "ns3/log.h"
#include "ns3/ndn-forwarding-strategy.h"
#include "ns3/ndn-l3-protocol.h"
                
namespace ns3 {
namespace ndn {
namespace fw {

typedef ForwardingStrategy BaseStrategy;

class CustomStrategy:
    public BaseStrategy
{
public:
  static TypeId
  GetTypeId ();
        
  static std::string
  GetLogName ();
          
  CustomStrategy ();
        
protected:
  virtual bool
  DoPropagateInterest (Ptr<Face> incomingFace,
                       Ptr<const Interest> interest,
                       Ptr<pit::Entry> pitEntry);

protected:
  static LogComponent g_log;

public:
        
// private:
//   std::string m_variable;

private:

};
} // namespace fw
} // namespace ndn
} // namespace ns3
                
#endif // CUSTOM_STRATEGY_H
