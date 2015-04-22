/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * NORIA: Node Reservation Intelligent Agent
 * Author: Andres Mauricio Bejarano Posada <abejarano@uninorte.edu.co>
 *
 * Basado en build/ns3/net-device-container.h
 */

#ifndef NORIA_CONTAINER_H
#define NORIA_CONTAINER_H

#include "noria.h"
#include "ns3/internet-module.h"
#include <stdint.h>
#include <vector>

using namespace ns3;

/**
 *
 */
class NoriaContainer {

   public:
   
   /**
    *
    */
   NoriaContainer (void);
   
   /**
    *
    */
   void Add (Noria noria);
   
   /**
    *
    */
   void AddRelation (uint32_t index, Mac48Address mac, Ipv4Address ip);
   
   /**
    *
    */
   void AddReserved (uint32_t index, Ipv4Address ip, double time);
   
   /**
    *
    */
   void AddTransmission (uint32_t index, Ipv4Address ip, double time);
   
   /**
    *
    */
   void Clear (void);
   
   /**
    *
    */
   void EraseReservedNode (uint32_t index, uint32_t reservedIndex);
   
   /**
    *
    */
   void EraseTransmission (uint32_t index, Ipv4Address ip);
   
   /**
    *
    */
   bool ExistTransmission (uint32_t index, Ipv4Address ip);
   
   /**
    *
    */
   Noria Get (uint32_t i) const;
   
   /**
    *
    */
   Ipv4Address GetRelatedIpAddress (uint32_t index, Mac48Address mac);
   
   /**
    *
    */
   double GetRecalculateRoutingTable (uint32_t index);
   
   /**
    *
    */
   uint32_t GetSize (void) const;
   
   /**
    *
    */
   uint32_t GetNoriaIndex (Mac48Address mac);
   
   /**
    *
    */
   double GetRecalculateRoutingTablePeriod (uint32_t index);
   
   /**
    *
    */
   double GetTransmissionTime (uint32_t index, Ipv4Address ip);
   
   /**
    *
    */
   void Install (NodeContainer container, Ipv4InterfaceContainer interfaces);
   
   /**
    *
    */
   bool IsReserved (uint32_t index);
   
   /**
    *
    */
   void RecalculateRoutingTable (uint32_t index);
   
   /**
    *
    */
   void ReceivePacketProcess (uint32_t index, Ptr<Packet> packet);
   
   /**
    *
    */
   void Reserve (uint32_t index, double now, Ipv4Address srcIP, Ipv4Address dstIP);
   
   /**
    *
    */
   void Reset (uint32_t index);
   
   /**
    *
    */
   void SendPacketProcess (uint32_t index, Ptr<Packet> packet);
   
   /**
    *
    */
   void SetRecalculateRoutingTablePeriod (double period);
   
   /**
    *
    */
   void SetRecalculateRoutingTablePeriod (uint32_t index, double period);
   
   /**
    *
    */
   void ShowConfig (void);
   
   /**
    *
    */
   void UpdateReservedNodes (uint32_t index, double time);
   
   /**
    *
    */
   void UpdateTransmissionTime (uint32_t index, Ipv4Address ip, double time);
   
   /**
    *
    */
   void WriteAddressRelations (bool enable);
   
   /**
    *
    */
   void WriteReservationState (bool enable);
   
   /**
    *
    */
   void WriteReservationState (uint32_t index);
   
   /**
    *
    */
   void WriteReservedNodes (bool enable);
   
   /**
    *
    */
   void WriteRoutingTable (uint32_t index);
   
   /**
    *
    */
   void WriteRoutingTables (bool enable);
   
   
   private:
   
   /**
    *
    */
   std::vector<Noria> m_norias;

};

#endif

