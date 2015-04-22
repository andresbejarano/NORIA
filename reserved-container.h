/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * NORIA: Node Reservation Intelligent Agent
 * Author: Andres Mauricio Bejarano Posada <abejarano@uninorte.edu.co>
 */

#ifndef RESERVED_CONTAINER_H
#define RESERVED_CONTAINER_H

using namespace ns3;

/**
 * Struct where the info of the reserved nodes is stored
 */
struct Reserved {

   /**
    * The IP address of the reserved node
    */
   Ipv4Address r_ip;
   
   /**
    * Time (in seconds) whene the node was identified as reserved
    */
   double r_time;
   
   /**
    * Reserved registry creator
    */
   Reserved (Ipv4Address ip, double time) {
      r_ip.Set(ip.Get());
      r_time = time;
   };
};

/**
 *
 */
class ReservedContainer {

   public:
   
   /**
    *
    */
   ReservedContainer (void);
   
   /**
    *
    */
   void Add (Ipv4Address ip, double time);
   
   /**
    *
    */
   void Erase (uint32_t index);
   
   /**
    *
    */
   bool Exist (Ipv4Address ip);
   
   /**
    *
    */
   Ipv4Address GetIp (uint32_t index);
   
   /**
    *
    */
   uint32_t GetSize (void);
   
   /**
    *
    */
   double GetTime (uint32_t index);
   
   /**
    *
    */
   double GetTime (Ipv4Address ip);
   
   /**
    *
    */
   void Update (double time);
   
   /**
    *
    */
   void Write (void);
   
   /**
    *
    */
   void Write (std::string fileName);
   
   
   private:
   
   /**
    *
    */
   std::vector<Reserved> v_reserved;
   
};

#endif

