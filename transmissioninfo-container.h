/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * NORIA: Node Reservation Intelligent Agent
 * Author: Andres Mauricio Bejarano Posada <abejarano@uninorte.edu.co>
 */

#ifndef TRANSMISSIONINFO_CONTAINER_H
#define TRANSMISSIONINFO_CONTAINER_H

using namespace ns3;

/**
 * Struct where the info of the transmissions is stored
 */
struct TransmissionInfo {

   /**
    * The IP address of the destination node
    */
   Ipv4Address t_dstIp;
   
   /**
    * Time (in seconds) of the last packet sent to the destination node
    */
   double t_time;
   
   /**
    * Transmission info registry creator
    */
   TransmissionInfo (Ipv4Address ip, double time) {
      t_dstIp.Set(ip.Get());
      t_time = time;
   };
};


/**
 *
 */
class TransmissionInfoContainer {

   public:
   
   /**
    *
    */
   TransmissionInfoContainer ();
   
   /**
    *
    */
   void Add (Ipv4Address ip, double time);
   
   /**
    *
    */
   void Erase (Ipv4Address ip);
   
   /**
    *
    */
   bool Exist (Ipv4Address ip);
   
   /**
    *
    */
   double GetTime (Ipv4Address ip);
   
   /**
    *
    */
   void Update (Ipv4Address ip, double time);
   
   /**
    *
    */
   void Write (void);
   
   private:
   
   /**
    *
    */
   std::vector<TransmissionInfo> v_transmissions;
   
};

#endif

