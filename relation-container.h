/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * NORIA: Node Reservation Intelligent Agent
 * Author: Andres Mauricio Bejarano Posada <abejarano@uninorte.edu.co>
 */

#ifndef RELATION_CONTAINER_H
#define RELATION_CONTAINER_H

using namespace ns3;

/**
 * Struct where the address relations is stored
 */
struct AddressRelation {

   /**
    * The MAC address of the relation
    */
   Mac48Address r_mac;
   
   /**
    * The IP address of the relation
    */
   Ipv4Address r_ip;
   
   /**
    * Address relation creator
    */
   AddressRelation (Mac48Address mac, Ipv4Address ip) {
      r_mac = Mac48Address::ConvertFrom (mac);
      r_ip.Set(ip.Get());
   };
};


/**
 * Class that manages the address relations
 */
class RelationContainer {

   public:
   
   /**
    *
    */
   RelationContainer (void);
   
   /**
    *
    */
   void Add (Mac48Address mac, Ipv4Address ip);
   
   /**
    *
    */
   bool Exist (Mac48Address mac);
   
   /**
    *
    */
   bool Exist (Ipv4Address ip);
   
   /**
    *
    */
   Ipv4Address GetIp (Mac48Address mac);
   
   /**
    *
    */
   Mac48Address GetMac (Ipv4Address ip);
   
   /**
    *
    */
   uint32_t GetSize (void);
   
   /**
    *
    */
   bool Related (Mac48Address mac, Ipv4Address ip);
   
   /**
    *
    */
   void Show (void);
   
   /**
    *
    */
   void UpdateRelation (Mac48Address mac, Ipv4Address ip);
   
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
   std::vector<AddressRelation> v_relations;

};

#endif

