/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * NORIA: Node Reservation Intelligent Agent
 * Author: Andres Mauricio Bejarano Posada <abejarano@uninorte.edu.co>
 */

#ifndef NORIA_H
#define NORIA_H

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/wifi-module.h"
#include "ns3/ipv4-l3-protocol.h"
#include "relation-container.h"
#include "reserved-container.h"
#include "transmissioninfo-container.h"
#include "olsr-routing-protocol.h"
#include <stdint.h>
#include <vector>

using namespace ns3;

/**
 *
 */
class Noria {

   public:
   
   /**
    *
    */
   Noria (void);
   
   /**
    *
    */
   void AddRelation (Mac48Address mac, Ipv4Address ip);
   
   /**
    *
    */
   void AddReserved (Ipv4Address ip, double time);
   
   /**
    *
    */
   void AddTransmission (Ipv4Address ip, double time);
   
   /**
    *
    */
   void EraseReservedNode (uint32_t reservedIndex);
   
   /**
    *
    */
   void EraseTransmission (Ipv4Address ip);
   
   /**
    *
    */
   bool ExistTransmission (Ipv4Address ip);
   
   /**
    *
    */
   uint32_t GetIndex (void);
   
   /**
    *
    */
   Ipv4Address GetIpAddress (void);
   
   /**
    *
    */
   Mac48Address GetMacAddress (void);
   
   /**
    *
    */
   double GetRecalculateRoutingTablePeriod (void);
   
   /**
    *
    */
   Ipv4Address GetRelatedIpAddress (Mac48Address mac);
   
   /**
    *
    */
   double GetTransmissionTime (Ipv4Address ip);
   
   /**
    *
    */
   void Install (uint32_t index, Ipv4Address ip);
   
   /**
    *
    */
   bool IsReserved (void);
   
   /**
    *
    */
   void RecalculateRoutingTable (void);
   
   /**
    *
    */
   void ReceivePacketProcess (Ptr<Packet> packet);
   
   /**
    *
    */
   void Reserve (double t, Ipv4Address src, Ipv4Address dst);
   
   /**
    *
    */
   bool Reserved (void);
   
   /**
    *
    */
   void Reset (void);
   
   /**
    *
    */
   void SendPacketProcess (Ptr<Packet> packet);
   
   /**
    *
    */
   void SetRecalculateRoutingTablePeriod (double period);
   
   /**
    *
    */
   void ShowConfig (void);
   
   /**
    *
    */
   void ShowRelations (void);
   
   /**
    *
    */
   void UpdateReservedNodes (double time);
   
   /**
    *
    */
   void UpdateTransmission (Ipv4Address ip, double time);
   
   /**
    *
    */
   void WriteConfig (void);
   
   /**
    *
    */
   void WriteAddressRelations (bool enable);
   
   /**
    *
    */
   void WriteAddressRelations (void);
   
   /**
    *
    */
   void WriteReservationState (bool enable);
   
   /**
    *
    */
   void WriteReservationState (void);
   
   /**
    *
    */
   void WriteReservedNodes (bool enable);
   
   /**
    *
    */
   void WriteReservedNodes (void);
   
   /**
    *
    */
   void WriteRoutingTable (bool enable);
   
   /**
    *
    */
   void WriteRoutingTable (void);
   
   /**
    *
    */
   void WriteTransmissions (void);
   
   
   private:
   
   /**
    *
    */
   uint32_t n_index;
   
   /**
    *
    */
   Ptr<WifiNetDevice> n_netDevice;
   
   /**
    *
    */
   Ipv4Address n_ipAddress;
   
   /**
    *
    */
   Ptr<olsr::RoutingProtocol> n_olsr;
   
   /**
    *
    */
   bool n_reserved;
   
   /**
    *
    */
   double n_reservedTime;
   
   /**
    *
    */
   Ipv4Address n_rSrcAddress;
   
   /**
    *
    */
   Ipv4Address n_rDstAddress;
   
   /**
    *
    */
   double n_recalculateRoutingTablePeriod;
   
   /**
    *
    */
   RelationContainer n_relations;
   
   /**
    *
    */
   ReservedContainer n_reservednodes;
   
   /**
    *
    */
   TransmissionInfoContainer n_transmissions;
   
   /**
    *
    */
   bool n_writeAddressRelations;
   
   /**
    *
    */
   bool n_writeReservationState;
   
   /**
    *
    */
   bool n_writeReservedNodes;
   
   /**
    *
    */
   bool n_writeRoutingTable;

};

#endif

