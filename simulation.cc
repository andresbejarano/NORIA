/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * NORIA: Node Reservation Intelligent Agent
 * Author: Andres Mauricio Bejarano Posada <abejarano@uninorte.edu.co>
 */

#include "noria.h"
#include "commons.h"
#include "noria-container.h"
#include "noria-simulation.h"
#include <ns3/flow-monitor-helper.h>
#include <iomanip>

#define OLSR_WILL_NEVER 0

using namespace ns3;

std::string filePrefix = "";

const std::string Commons::asciiRegistry     = "asciiRegistry";
const std::string Commons::dataRegistry      = "dataRegistry.csv";
const std::string Commons::flowRegistry      = "flowRegistry.xml";
const std::string Commons::mobilityRegistry  = "mobilityRegistry.mob";
const std::string Commons::noriaRegistry     = "noriaRegistry.csv";
const std::string Commons::pcapRegistry      = "pcapRegistry";
const std::string Commons::receivedRegistry  = "receivedRegistry.csv";
const std::string Commons::relationsRegistry = "relationsRegistry.txt";
const std::string Commons::reservedRegistry  = "reservedRegistry.txt";
const std::string Commons::sentRegistry      = "sentRegistry.csv";
const std::string Commons::stateRegistry     = "stateRegistry.csv";

const std::string Commons::csvSymbol         = ";";
const double Commons::RESERVED_LIFETIME      = 2;
const uint16_t Commons::OLSR_PORT            = 698;

NS_LOG_COMPONENT_DEFINE("noria-simulation");

// Contenedor de los Noria
NoriaContainer norias;

/**
 *
 */
static void RecalculateTable (uint32_t index) {
   double period = norias.GetRecalculateRoutingTablePeriod (index);     // Se obtiene el periodo (en segundos) en el que se debe recalcular la tabla de enrutamiento
   norias.RecalculateRoutingTable (index);                              // Se recalcula la tabla de enrutamiento
   Simulator::Schedule (Seconds (period), &RecalculateTable, index);    // Se indica que se debe volver a recalcular dentro del tiempo indicado
}

/**
 * Static procedure called by callback when a net device receive a packet
 */
static void ReceivePacket (Ptr<WifiNetDevice> device, Ptr<const Packet> packet, uint16_t channelFreqMhz, uint16_t channelNumber, uint32_t rate, bool isShortPreamble, double signalDbm, double noiseDbm) {
   Mac48Address address = Mac48Address::ConvertFrom (device->GetAddress ());
   uint32_t index = norias.GetNoriaIndex (address);
   if (index < norias.GetSize ()) {
      norias.ReceivePacketProcess (index, packet->Copy ());
   }
}

/**
 * Static procedure called by callback when a Noria have to reset its reservation info
 */
static void ResetNoria (uint32_t index) {
   norias.Reset (index);
}

/**
 * Static procedure called by callback when a net device sent a packet
 */
static void SendPacket (Ptr<WifiNetDevice> device, Ptr<const Packet> packet, uint16_t channelFreqMhz, uint16_t channelNumber, uint32_t rate, bool isShortPreamble) {
   Mac48Address address = Mac48Address::ConvertFrom (device->GetAddress ());
   uint32_t index = norias.GetNoriaIndex (address);
   if (index < norias.GetSize ()) {
      norias.SendPacketProcess (index, packet->Copy ());
   }
}

/**
 * Static procedure called by callback when a Noria needs to update its reserved registry
 */
static void UpdateReserved (uint32_t index) {
   double now = Simulator::Now ().GetSeconds ();
   norias.UpdateReservedNodes (index, now);
   Simulator::Schedule (Seconds (Commons::RESERVED_LIFETIME), &UpdateReserved, index);
}

/**
 * Static procedure called by callback when a Noria have to write in file its reservation state
 */
/*
static void WriteStates (uint32_t index) {
   norias.WriteReservationState (index);
}
*/


/**
 * +-------------------------------------------------------------------------+
 * | Clase RelationContainer                                                 |
 * +-------------------------------------------------------------------------+
 */

/**
 *
 */
RelationContainer::RelationContainer (void) {
}

/**
 *
 */
void RelationContainer::Add (Mac48Address mac, Ipv4Address ip) {
   if (!Exist (mac)) {
      AddressRelation a (mac, ip);
      v_relations.push_back (a);
   }
}

/**
 *
 */
bool RelationContainer::Exist (Mac48Address mac) {
   uint32_t n = v_relations.size();
   uint32_t i = 0;
   bool found = false;
   while (!found && i < n) {
      if (v_relations[i].r_mac == mac) {
         found = true;
      }
      else {
         i += 1;
      }
   }
   return found;
}

/**
 *
 */
bool RelationContainer::Exist (Ipv4Address ip) {
   uint32_t n = v_relations.size();
   uint32_t i = 0;
   bool found = false;
   while (!found && i < n) {
      if (v_relations[i].r_ip == ip) {
         found = true;
      }
      else {
         i += 1;
      }
   }
   return found;
}

/**
 *
 */
Ipv4Address RelationContainer::GetIp (Mac48Address mac) {
   uint32_t n = v_relations.size();
   uint32_t i = 0;
   bool found = false;
   Ipv4Address ip;
   while (!found && i < n) {
      if (v_relations[i].r_mac == mac) {
         found = true;
         ip.Set(v_relations[i].r_ip.Get());
      }
      else {
         i += 1;
      }
   }
   return ip;
}

/**
 *
 */
Mac48Address RelationContainer::GetMac (Ipv4Address ip) {
   uint32_t n = v_relations.size();
   uint32_t i = 0;
   bool found = false;
   Mac48Address mac;
   while (!found && i < n) {
      if (v_relations[i].r_ip == ip) {
         found = true;
         mac = Mac48Address::ConvertFrom (v_relations[i].r_mac);
      }
      else {
         i += 1;
      }
   }
   return mac;
}

/**
 *
 */
uint32_t RelationContainer::GetSize (void) {
   return v_relations.size();
}

/**
 *
 */
bool RelationContainer::Related (Mac48Address mac, Ipv4Address ip) {
   bool related = false;
   if (ip == GetIp(mac)) {
      related = true;
   }
   return related;
}

/**
 *
 */
void RelationContainer::Show (void) {
   uint32_t n = v_relations.size();
   for (uint32_t i = 0; i < n; i++) {
      std::cout << "-> Mac: " << v_relations[i].r_mac 
                << " IP: " << v_relations[i].r_ip 
                << std::endl;
   }
}

/**
 *
 */
void RelationContainer::UpdateRelation (Mac48Address mac, Ipv4Address ip) {
   uint32_t n = v_relations.size();
   uint32_t i = 0;
   bool found = false;
   while (!found && i < n) {
      if (v_relations[i].r_mac == mac) {
         found = true;
         v_relations[i].r_ip.Set (ip.Get());
      }
      else {
         i += 1;
      }
   }
}

/**
 *
 */
void RelationContainer::Write (void) {
   uint32_t n = v_relations.size();
   std::ofstream write((filePrefix + Commons::relationsRegistry).c_str(), std::ios::app);
   for (uint32_t i = 0; i < n; i++) {
      write << "-> MAC: " << v_relations[i].r_mac << " IP: " << v_relations[i].r_ip << std::endl;
   }
   write << std::endl;
   write.close();
}

/**
 *
 */
void RelationContainer::Write (std::string fileName) {
   uint32_t n = v_relations.size();
   std::ofstream write(fileName.c_str(), std::ios::app);
   for (uint32_t i = 0; i < n; i++) {
      write << "-> MAC: " << v_relations[i].r_mac << " IP: " << v_relations[i].r_ip << std::endl;
   }
   write << std::endl;
   write.close();
}

/**
 * +-------------------------------------------------------------------------+
 * | Clase ReservedContainer                                                 |
 * +-------------------------------------------------------------------------+
 */

/**
 *
 */
ReservedContainer::ReservedContainer (void) {
}

/**
 *
 */
void ReservedContainer::Add (Ipv4Address ip, double time) {
   if (!Exist (ip)) {
      Reserved r (ip, time);
      v_reserved.push_back (r);
   }
}

/**
 *
 */
void ReservedContainer::Erase (uint32_t index) {
   v_reserved.erase (v_reserved.begin() + index);
}

/**
 *
 */
bool ReservedContainer::Exist (Ipv4Address ip) {
   uint32_t n = v_reserved.size();
   uint32_t i = 0;
   bool found = false;
   while (!found && i < n) {
      if (v_reserved[i].r_ip == ip) {
         found = true;
      }
      else {
         i += 1;
      }
   }
   return found;
}

/**
 *
 */
Ipv4Address ReservedContainer::GetIp (uint32_t index) {
   Ipv4Address ip;
   ip.Set(v_reserved[index].r_ip.Get());
   return ip;
}

/**
 *
 */
uint32_t ReservedContainer::GetSize (void) {
   return v_reserved.size();
}

/**
 *
 */
double ReservedContainer::GetTime (uint32_t index) {
   return v_reserved[index].r_time;
}

/**
 *
 */
double ReservedContainer::GetTime (Ipv4Address ip) {
   uint32_t n = v_reserved.size();
   uint32_t i = 0;
   bool found = false;
   double time = 0;
   while (!found && i < n) {
      if (v_reserved[i].r_ip == ip) {
         found = true;
         time = v_reserved[i].r_time;
      }
      else {
         i += 1;
      }
   }
   return time;
}

/**
 *
 */
void ReservedContainer::Update (double time) {
   uint32_t n = v_reserved.size();
   std::vector<Reserved> temp;
   for (uint32_t i = 0;i < n;i += 1) {
      if (time - v_reserved[i].r_time < Commons::RESERVED_LIFETIME) {
         Reserved r (v_reserved[i].r_ip, v_reserved[i].r_time);
         temp.push_back (r);
      }
   }
   v_reserved = temp;
}

/**
 *
 */
void ReservedContainer::Write (void) {
   uint32_t n = v_reserved.size();
   std::ofstream write((filePrefix + Commons::reservedRegistry).c_str(), std::ios::app);
   for (uint32_t i = 0; i < n; i++) {
      write << "-> IP: " << v_reserved[i].r_ip << " TIME: " << v_reserved[i].r_time << std::endl;
   }
   write << std::endl;
   write.close();
}

/**
 *
 */
void ReservedContainer::Write (std::string fileName) {
   uint32_t n = v_reserved.size();
   std::ofstream write(fileName.c_str(), std::ios::app);
   for (uint32_t i = 0; i < n; i++) {
      write << "-> IP: " << v_reserved[i].r_ip << " TIME: " << v_reserved[i].r_time << std::endl;
   }
   write << std::endl;
   write.close();
}

/**
 * +-------------------------------------------------------------------------+
 * | Clase TransmissionInfoContainer                                         |
 * +-------------------------------------------------------------------------+
 */


/**
 *
 */
TransmissionInfoContainer::TransmissionInfoContainer () {
}

/**
 *
 */
void TransmissionInfoContainer::Add (Ipv4Address ip, double time) {
   if (!Exist (ip)) {
      TransmissionInfo t (ip, time);
      v_transmissions.push_back (t);
   }
   else {
      Update (ip, time);
   }
}

/**
 *
 */
void TransmissionInfoContainer::Erase (Ipv4Address ip) {
   uint32_t n = v_transmissions.size();
   std::vector<TransmissionInfo> temp;
   for (uint32_t i = 0;i < n;i += 1) {
      if (v_transmissions[i].t_dstIp != ip) {
         TransmissionInfo t (v_transmissions[i].t_dstIp, v_transmissions[i].t_time);
         temp.push_back(t);
      }
   }
   v_transmissions = temp;
}

/**
 *
 */
bool TransmissionInfoContainer::Exist (Ipv4Address ip) {
   uint32_t n = v_transmissions.size();
   uint32_t i = 0;
   bool found = false;
   while (!found && i < n) {
      if (v_transmissions[i].t_dstIp == ip) {
         found = true;
      }
      else {
         i += 1;
      }
   }
   return found;
}

/**
 *
 */
double TransmissionInfoContainer::GetTime (Ipv4Address ip) {
   uint32_t n = v_transmissions.size();
   uint32_t i = 0;
   bool found = false;
   double time = 0;
   while (!found && i < n) {
      if (v_transmissions[i].t_dstIp == ip) {
         found = true;
         time = v_transmissions[i].t_time;
      }
      else {
         i += 1;
      }
   }
   return time;
}

/**
 *
 */
void TransmissionInfoContainer::Update (Ipv4Address ip, double time) {
   uint32_t n = v_transmissions.size();
   uint32_t i = 0;
   bool found = false;
   while (!found && i < n) {
      if (v_transmissions[i].t_dstIp == ip) {
         found = true;
         if (v_transmissions[i].t_time < time) {
            v_transmissions[i].t_time = time;
         }
      }
      else {
         i += 1;
      }
   }
}

/**
 *
 */
void TransmissionInfoContainer::Write (void) {
}

/**
 * +-------------------------------------------------------------------------+
 * | Clase Noria                                                             |
 * +-------------------------------------------------------------------------+
 */

/**
 *
 */
Noria::Noria () : n_reserved (false), n_reservedTime (0) {
}

/**
 *
 */
void Noria::AddRelation (Mac48Address mac, Ipv4Address ip) {
   n_relations.Add (mac, ip);
}

/**
 *
 */
void Noria::AddReserved (Ipv4Address ip, double time) {
   n_reservednodes.Add (ip, time);
}

/**
 *
 */
void Noria::AddTransmission (Ipv4Address ip, double time) {
   n_transmissions.Add (ip, time);
}

/**
 *
 */
void Noria::EraseReservedNode (uint32_t reservedIndex) {
   n_reservednodes.Erase (reservedIndex);
}

/**
 *
 */
void Noria::EraseTransmission (Ipv4Address ip) {
   n_transmissions.Erase (ip);
}

/**
 *
 */
bool Noria::ExistTransmission (Ipv4Address ip) {
   return n_transmissions.Exist (ip);
}


/**
 *
 */
uint32_t Noria::GetIndex () {
   return n_index;
}

/**
 *
 */
Ipv4Address Noria::GetIpAddress () {
   Ipv4Address ip;
   ip.Set(n_ipAddress.Get());
   return ip;
}

/**
 *
 */
Mac48Address Noria::GetMacAddress () {
   Mac48Address mac = Mac48Address::ConvertFrom(n_netDevice->GetAddress());
   return mac;
}

/**
 *
 */
double Noria::GetRecalculateRoutingTablePeriod (void) {
   return n_recalculateRoutingTablePeriod;
}

/**
 *
 */
Ipv4Address Noria::GetRelatedIpAddress (Mac48Address mac) {
   return n_relations.GetIp (mac);
}

/**
 *
 */
double Noria::GetTransmissionTime (Ipv4Address ip) {
   return n_transmissions.GetTime (ip);
}

/**
 * Basade en: src/wifi/helper/yans-wifi-helper.cc
 * Funcion: EnablePcapInternal(std::string, Ptr<NetDevice>, bool, bool)
 * Aca se define la funcion que recibira todos los paquetes que se 
 * escuchan en los dispositivos de red, para luego ser enviados a los 
 * respectivos agentes.
 */
void Noria::Install (uint32_t index, Ipv4Address ip) {
   n_index = index;     // Indice del agente segun el orden de instalacion
   
   Config::MatchContainer match;        //--------------------------------------
   std::stringstream noriaIndex;        //--------------------------------------
   noriaIndex << index;                 //--------------------------------------
   
   std::string netDevicePath = "/NodeList/" + noriaIndex.str() + "/DeviceList/0/$ns3::WifiNetDevice";   // Camino de consulta de los dispositivos de red
   match = Config::LookupMatches (netDevicePath.c_str());                                               // Buscador de correspondencias
   n_netDevice = match.Get(0)->GetObject<WifiNetDevice>();                                              // Obtencion del dispositivo de red
   
   Ptr<WifiPhy> phy = n_netDevice->GetPhy ();                                                                   // Capa fisica del dispositivo de red
   phy->TraceConnectWithoutContext ("MonitorSnifferRx", MakeBoundCallback (&ReceivePacket, n_netDevice));       // Establecimiento del modo promiscuo
   phy->TraceConnectWithoutContext ("MonitorSnifferTx", MakeBoundCallback (&SendPacket, n_netDevice));
   
   n_ipAddress.Set(ip.Get());   // Indicacion de la direccion IP del nodo
   
   std::string olsrPath = "/NodeList/" + noriaIndex.str() + "/$ns3::olsr::RoutingProtocol";     // Camino de consulta al protocolo OLSR
   match = Config::LookupMatches (olsrPath.c_str());                                            // Buscador de correspondencias
   n_olsr = match.Get(0)->GetObject<olsr::RoutingProtocol>();                                   // Obtencion del protocolo OLSR
   
   n_relations.Add (Mac48Address::ConvertFrom(n_netDevice->GetAddress()), ip);
   n_writeAddressRelations = false;
   n_writeReservationState = false;
   n_writeReservedNodes = false;
   n_writeRoutingTable = false;
   WriteConfig();
   
   Simulator::Schedule (Seconds (Commons::RESERVED_LIFETIME), &UpdateReserved, n_index);
   //Simulator::Schedule (Seconds (Commons::RESERVED_LIFETIME), &RecalculateTable, n_index);
}

/**
 *
 */
bool Noria::IsReserved (void) {
   return n_reserved;
}

/**
 *
 */
void Noria::ReceivePacketProcess (Ptr<Packet> packet) {
   double now = Simulator::Now().GetSeconds();        //Tiempo de simulacion (en segundos)
   
   /* WifiMac Header */
   WifiMacHeader macHeader;             //Formato de cabecera WifiMAC
   packet->RemoveHeader(macHeader);     //Obtener los datos de la cabecera WifiMAC
   if (!macHeader.IsAck()) {            //Si no es un ACK
   
      /* LLC/SNAP Header */
      LlcSnapHeader llcHeader;                  //Formato de cabecera LLC
      if (packet->PeekHeader(llcHeader) > 0) {  //Si el paquete contiene cabecera LLC/SNAP (Formalismo, pues si lo debe tener)
         packet->RemoveHeader(llcHeader);       //Obtener los datos de la cabecera LLS/SNAP
      }
      
      /* ARP Header */
      ArpHeader arpHeader;                      //Formato de cabecera ARP
      if (packet->PeekHeader(arpHeader) == 0) { //Si el paquete no contiene una cabecera ARP
         
         /* IPv4 Header */
         Ipv4Header ipv4Header;                         //Formato de cabecera IPv4
         if (packet->PeekHeader(ipv4Header) > 0) {      //Si el paquete contiene cabecera IPv4 (Formalismo, pues si lo debe tener)
            packet->RemoveHeader(ipv4Header);           //Obtener los datos de la cabecera IPv4
         }
         
         /* UDP Header */
         UdpHeader udpHeader;
         if (packet->PeekHeader(udpHeader) > 0) {            //Si el paquete contiene cabecera UDP
         
            Mac48Address nodeMAC = GetMacAddress();          //Direccion MAC del dispositivo de red del nodo
            Mac48Address srcMAC = macHeader.GetAddr2();      //Source Address (IEEE 802.11 MAC header)
            Mac48Address dstMAC = macHeader.GetAddr1();      //Destination Address (IEEE 802.11 MAC header)
            Ipv4Address srcIP = ipv4Header.GetSource();      //Direccion IP origen de la transmision
            Ipv4Address dstIP = ipv4Header.GetDestination(); //Direccion IP destino de la transmision
            
            //Si el puerto UDP destino del paquete no es el mismo del protocolo OLSR
            if (udpHeader.GetDestinationPort() != Commons::OLSR_PORT) { 
               
               /* Estos son los paquetes que se deben procesar por el Noria para identificar la reserva */
               //Si la direccion MAC del dispositivo es igual a la direccion destino del paquete
               if (nodeMAC == dstMAC) { 
                  
                  /* El nodo recibio el paquete (es el destino a nivel de la capa de enlace) */
                  //¿El nodo no es el destino de la transmision? (Es un nodo intermedio)
                  if (srcIP != GetIpAddress()) {
                     
                     //El nodo se encuentra reservado
                     if (norias.IsReserved (GetIndex())) {
                        
                        //El nodo esta reservado
                        if (!(srcIP == n_rSrcAddress && dstIP == n_rDstAddress)) {
                           
                           //Las direcciones IP origen y destino del paquete no son iguales a las direcciones de la reserva
                           //discard(packet);
                        }
                     }
                     else {
                        
                        //El nodo no esta reservado
                        //El nodo no se encuentra reservado, por lo que su estado cambia a reservado
                        norias.Reserve (GetIndex(), now, srcIP, dstIP);
                        if (n_writeReservationState) {
                           norias.WriteReservationState (GetIndex());
                        }
                     }
                  }
               }
               else {
                  
                  // El nodo escucho el paquete (no es el destino a nivel de la capa de enlace)
                  // ¿El nodo que recibio el paquete es el destino de la transmision?
                  if (!n_relations.Related (dstMAC, dstIP)) {
                     
                     // ¿El destino del paquete es igual al destino de algun paquete enviado?
                     if (!norias.ExistTransmission (GetIndex(), dstIP)) {
                        
                        // Reservar el nodo que recibio el paquete pues hace parte del intermedio de una transmision
                        Ipv4Address ip = n_relations.GetIp (dstMAC);
                        if (n_relations.Exist (ip)) {
                           norias.AddReserved (GetIndex(), ip, now);
                           //norias.RecalculateRoutingTable (GetIndex());
                           if (n_writeReservedNodes) {
                              norias.WriteReservedNodes (GetIndex());
                           }
                           if (n_writeRoutingTable) {
                              norias.WriteRoutingTable (GetIndex());
                           }
                        }
                     }
                  }
               }
            }
            else {
               
               /* Se añade la informacion de las direcciones relacionadas al vector de relacion de direcciones del Noria */
               norias.AddRelation (GetIndex(), srcMAC, srcIP);
               if (n_writeAddressRelations) {
                  norias.WriteAddressRelations (GetIndex());
               }
            }
         }
      }
   }
}

/**
 * Recalculates the routing table according to the implementations of the OLSR's routing table calculation algorithm
 * found in src/olsr/model/olsr-routing-protocol.cc
 * Every timen when a new entre is going to be added in the routing table, a new condition is evaluated. It checks if the
 * r_next_addr is not in the list of the reserved nodes identified by the Noria. If the IP address is not registered as
 * reserved then the r_next_addr is added in the routing table as a next address, if not the entry is not added.
 */
void Noria::RecalculateRoutingTable () {
   NS_LOG_DEBUG (Simulator::Now().GetSeconds() << " s: Node " << n_olsr->m_mainAddress << ": RoutingTableComputation begin...");
   
   // 1. All the entries from the routing table are removed.
   n_olsr->Clear ();
   
   // 2. The new routing entries are added starting with the
   // symmetric neighbors (h=1) as the destination nodes.
   const NeighborSet &neighborSet = n_olsr->m_state.GetNeighbors ();
   for (NeighborSet::const_iterator it = neighborSet.begin (); it != neighborSet.end (); it++) {
      NeighborTuple const &nb_tuple = *it;
      NS_LOG_DEBUG ("Looking at neighbor tuple: " << nb_tuple);
      if (nb_tuple.status == NeighborTuple::STATUS_SYM) {
         bool nb_main_addr = false;
         const LinkTuple *lt = NULL;
         const LinkSet &linkSet = n_olsr->m_state.GetLinks ();
         for (LinkSet::const_iterator it2 = linkSet.begin (); it2 != linkSet.end (); it2++) {
            LinkTuple const &link_tuple = *it2;
            NS_LOG_DEBUG ("Looking at link tuple: " << link_tuple << (link_tuple.time >= Simulator::Now () ? "" : " (expired)"));
            if ((n_olsr->GetMainAddress (link_tuple.neighborIfaceAddr) == nb_tuple.neighborMainAddr) && link_tuple.time >= Simulator::Now ()) {
               NS_LOG_LOGIC ("Link tuple matches neighbor " << nb_tuple.neighborMainAddr << " => adding routing table entry to neighbor");
               lt = &link_tuple;
               n_olsr->AddEntry (link_tuple.neighborIfaceAddr, link_tuple.neighborIfaceAddr, link_tuple.localIfaceAddr, 1);
               if (link_tuple.neighborIfaceAddr == nb_tuple.neighborMainAddr) {
                  nb_main_addr = true;
               }
            }
            else {
               NS_LOG_LOGIC ("Link tuple: linkMainAddress= " << n_olsr->GetMainAddress (link_tuple.neighborIfaceAddr)
                                                             << "; neighborMainAddr =  " << nb_tuple.neighborMainAddr
                                                             << "; expired=" << int (link_tuple.time < Simulator::Now ())
                                                             << " => IGNORE");
            }
         }
         
         // If, in the above, no R_dest_addr is equal to the main
         // address of the neighbor, then another new routing entry
         // with MUST be added, with:
         //      R_dest_addr  = main address of the neighbor;
         //      R_next_addr  = L_neighbor_iface_addr of one of the
         //                     associated link tuple with L_time >= current time;
         //      R_dist       = 1;
         //      R_iface_addr = L_local_iface_addr of the
         //                     associated link tuple.
         if (!nb_main_addr && lt != NULL) {
            NS_LOG_LOGIC ("no R_dest_addr is equal to the main address of the neighbor "
                            "=> adding additional routing entry");
            n_olsr->AddEntry (nb_tuple.neighborMainAddr, lt->neighborIfaceAddr, lt->localIfaceAddr, 1);
         }
      }
   }
   
   //  3. for each node in N2, i.e., a 2-hop neighbor which is not a
   //  neighbor node or the node itself, and such that there exist at
   //  least one entry in the 2-hop neighbor set where
   //  N_neighbor_main_addr correspond to a neighbor node with
   //  willingness different of WILL_NEVER,
   const TwoHopNeighborSet &twoHopNeighbors = n_olsr->m_state.GetTwoHopNeighbors ();
   for (TwoHopNeighborSet::const_iterator it = twoHopNeighbors.begin (); it != twoHopNeighbors.end (); it++) {
      TwoHopNeighborTuple const &nb2hop_tuple = *it;
      NS_LOG_LOGIC ("Looking at two-hop neighbor tuple: " << nb2hop_tuple);
      
      // a 2-hop neighbor which is not a neighbor node or the node itself
      if (n_olsr->m_state.FindSymNeighborTuple (nb2hop_tuple.twoHopNeighborAddr)) {
         NS_LOG_LOGIC ("Two-hop neighbor tuple is also neighbor; skipped.");
         continue;
      }
      if (nb2hop_tuple.twoHopNeighborAddr == n_olsr->m_mainAddress) {
         NS_LOG_LOGIC ("Two-hop neighbor is self; skipped.");
         continue;
      }
      
      // ...and such that there exist at least one entry in the 2-hop
      // neighbor set where N_neighbor_main_addr correspond to a
      // neighbor node with willingness different of WILL_NEVER...
      bool nb2hopOk = false;
      for (NeighborSet::const_iterator neighbor = neighborSet.begin (); neighbor != neighborSet.end (); neighbor++) {
         if (neighbor->neighborMainAddr == nb2hop_tuple.neighborMainAddr && neighbor->willingness != OLSR_WILL_NEVER) {
            nb2hopOk = true;
            break;
         }
      }
      if (!nb2hopOk) {
         NS_LOG_LOGIC ("Two-hop neighbor tuple skipped: 2-hop neighbor "
                        << nb2hop_tuple.twoHopNeighborAddr
                        << " is attached to neighbor " << nb2hop_tuple.neighborMainAddr
                        << ", which was not found in the Neighbor Set.");
         continue;
      }

      // one selects one 2-hop tuple and creates one entry in the routing table with:
      //                R_dest_addr  =  the main address of the 2-hop neighbor;
      //                R_next_addr  = the R_next_addr of the entry in the
      //                               routing table with:
      //                                   R_dest_addr == N_neighbor_main_addr
      //                                                  of the 2-hop tuple;
      //                R_dist       = 2;
      //                R_iface_addr = the R_iface_addr of the entry in the
      //                               routing table with:
      //                                   R_dest_addr == N_neighbor_main_addr
      //                                                  of the 2-hop tuple;
      RoutingTableEntry entry;
      bool foundEntry = n_olsr->Lookup (nb2hop_tuple.neighborMainAddr, entry);
      if (foundEntry && !n_reservednodes.Exist(entry.nextAddr)) { // MODIFICADO
         NS_LOG_LOGIC ("Adding routing entry for two-hop neighbor.");
         n_olsr->AddEntry (nb2hop_tuple.twoHopNeighborAddr, entry.nextAddr, entry.interface, 2);
      }
      else {
         NS_LOG_LOGIC ("NOT adding routing entry for two-hop neighbor (" << nb2hop_tuple.twoHopNeighborAddr << " not found in the routing table)");
      }
   }
   
   for (uint32_t h = 2;; h++) {
      bool added = false;

      // 3.1. For each topology entry in the topology table, if its
      // T_dest_addr does not correspond to R_dest_addr of any
      // route entry in the routing table AND its T_last_addr
      // corresponds to R_dest_addr of a route entry whose R_dist
      // is equal to h, then a new route entry MUST be recorded in
      // the routing table (if it does not already exist)
      const TopologySet &topology = n_olsr->m_state.GetTopologySet ();
      for (TopologySet::const_iterator it = topology.begin (); it != topology.end (); it++) {
         const TopologyTuple &topology_tuple = *it;
         NS_LOG_LOGIC ("Looking at topology tuple: " << topology_tuple);
         
         RoutingTableEntry destAddrEntry, lastAddrEntry;
         bool have_destAddrEntry = n_olsr->Lookup (topology_tuple.destAddr, destAddrEntry);
         bool have_lastAddrEntry = n_olsr->Lookup (topology_tuple.lastAddr, lastAddrEntry);
         if (!have_destAddrEntry && have_lastAddrEntry && lastAddrEntry.distance == h  && !n_reservednodes.Exist(lastAddrEntry.nextAddr)) { // MODIFICADO
            NS_LOG_LOGIC ("Adding routing table entry based on the topology tuple.");
            // then a new route entry MUST be recorded in
            //                the routing table (if it does not already exist) where:
            //                     R_dest_addr  = T_dest_addr;
            //                     R_next_addr  = R_next_addr of the recorded
            //                                    route entry where:
            //                                    R_dest_addr == T_last_addr
            //                     R_dist       = h+1; and
            //                     R_iface_addr = R_iface_addr of the recorded
            //                                    route entry where:
            //                                       R_dest_addr == T_last_addr.
            n_olsr->AddEntry (topology_tuple.destAddr, lastAddrEntry.nextAddr, lastAddrEntry.interface, h + 1);
            added = true;
         }
         else {
            NS_LOG_LOGIC ("NOT adding routing table entry based on the topology tuple: "
                            "have_destAddrEntry=" << have_destAddrEntry
                                                  << " have_lastAddrEntry=" << have_lastAddrEntry
                                                  << " lastAddrEntry.distance=" << (int) lastAddrEntry.distance
                                                  << " (h=" << h << ")");
         }
      }
      
      if (!added) break;
   }
   
   // 4. For each entry in the multiple interface association base
   // where there exists a routing entry such that:
   // R_dest_addr == I_main_addr (of the multiple interface association entry)
   // AND there is no routing entry such that:
   // R_dest_addr == I_iface_addr
   const IfaceAssocSet &ifaceAssocSet = n_olsr->m_state.GetIfaceAssocSet ();
   for (IfaceAssocSet::const_iterator it = ifaceAssocSet.begin (); it != ifaceAssocSet.end (); it++) {
      IfaceAssocTuple const &tuple = *it;
      RoutingTableEntry entry1, entry2;
      bool have_entry1 = n_olsr->Lookup (tuple.mainAddr, entry1);
      bool have_entry2 = n_olsr->Lookup (tuple.ifaceAddr, entry2);
      if (have_entry1 && !have_entry2 && !n_reservednodes.Exist(entry1.nextAddr)) {     // MODIFICADO
         // then a route entry is created in the routing table with:
         //       R_dest_addr  =  I_iface_addr (of the multiple interface
         //                                     association entry)
         //       R_next_addr  =  R_next_addr  (of the recorded route entry)
         //       R_dist       =  R_dist       (of the recorded route entry)
         //       R_iface_addr =  R_iface_addr (of the recorded route entry).
         n_olsr->AddEntry (tuple.ifaceAddr, entry1.nextAddr, entry1.interface, entry1.distance);
      }
   }
   
   // 5. For each tuple in the association set,
   //    If there is no entry in the routing table with:
   //        R_dest_addr     == A_network_addr/A_netmask
   //   and if the announced network is not announced by the node itself,
   //   then a new routing entry is created.
   const AssociationSet &associationSet = n_olsr->m_state.GetAssociationSet ();
   
   // Clear HNA routing table
   for (uint32_t i = 0; i < n_olsr->m_hnaRoutingTable->GetNRoutes (); i++) {
      n_olsr->m_hnaRoutingTable->RemoveRoute (0);
   }
   
   for (AssociationSet::const_iterator it = associationSet.begin (); it != associationSet.end (); it++) {
      AssociationTuple const &tuple = *it;
      
      // Test if HNA associations received from other gateways
      // are also announced by this node. In such a case, no route
      // is created for this association tuple (go to the next one).
      bool goToNextAssociationTuple = false;
      const Associations &localHnaAssociations = n_olsr->m_state.GetAssociations ();
      NS_LOG_DEBUG ("Nb local associations: " << localHnaAssociations.size ());
      for (Associations::const_iterator assocIterator = localHnaAssociations.begin (); assocIterator != localHnaAssociations.end (); assocIterator++) {
         Association const &localHnaAssoc = *assocIterator;
         if (localHnaAssoc.networkAddr == tuple.networkAddr && localHnaAssoc.netmask == tuple.netmask) {
            NS_LOG_DEBUG ("HNA association received from another GW is part of local HNA associations: no route added for network "
                            << tuple.networkAddr << "/" << tuple.netmask);
            goToNextAssociationTuple = true;
         }
      }
      if (goToNextAssociationTuple) {
         continue;
      }
      
      RoutingTableEntry gatewayEntry;
      bool gatewayEntryExists = n_olsr->Lookup (tuple.gatewayAddr, gatewayEntry);
      bool addRoute = false;
      uint32_t routeIndex = 0;
      for (routeIndex = 0; routeIndex < n_olsr->m_hnaRoutingTable->GetNRoutes (); routeIndex++) {
         Ipv4RoutingTableEntry route = n_olsr->m_hnaRoutingTable->GetRoute (routeIndex);
         if (route.GetDestNetwork () == tuple.networkAddr && route.GetDestNetworkMask () == tuple.netmask) {
            break;
         }
      }
      
      if (routeIndex == n_olsr->m_hnaRoutingTable->GetNRoutes ()) {
         addRoute = true;
      }
      else if (gatewayEntryExists && n_olsr->m_hnaRoutingTable->GetMetric (routeIndex) > gatewayEntry.distance) {
         n_olsr->m_hnaRoutingTable->RemoveRoute (routeIndex);
         addRoute = true;
      }
      
      if(addRoute && gatewayEntryExists) {
         n_olsr->m_hnaRoutingTable->AddNetworkRouteTo (tuple.networkAddr, tuple.netmask, gatewayEntry.nextAddr, gatewayEntry.interface, gatewayEntry.distance);
      }
   }
   
   NS_LOG_DEBUG ("Node " << n_olsr->m_mainAddress << ": RoutingTableComputation end.");
   n_olsr->m_routingTableChanged (n_olsr->GetSize ());
   
   //Indica cuando debe volver a calcular la tabla de enrutamiento
   //Simulator::Schedule (Seconds(Commons::RESERVED_LIFETIME), &RecalculateTable, GetIndex());
}

/**
 *
 */
void Noria::Reserve (double t, Ipv4Address src, Ipv4Address dst) {
   n_reserved = true;                   //Indica que el nodo esta reservado
   n_reservedTime = t;                  //Almacena el tiempo (en segundos) en que se hizo la reserva
   n_rSrcAddress.Set(src.Get());        //Almacena la direccion IP origen de la transmision
   n_rDstAddress.Set(dst.Get());        //Almacena la direccion IP destino de la transmision
   Simulator::Schedule (Seconds(Commons::RESERVED_LIFETIME), &ResetNoria, GetIndex());
}

/**
 *
 */
bool Noria::Reserved () {
   return n_reserved;
}

/**
 * Reset the variables of the reservation status
 */
void Noria::Reset () {
   n_reserved = false;                          //Reseteo del indicador del modo de reserva
   n_reservedTime = 0;                          //Resetea el tiempo de reserva que tenia anteriormente
   n_rSrcAddress = Ipv4Address::GetZero();      //Reseteo de la IP del nodo origen de la reserva
   n_rDstAddress = Ipv4Address::GetZero();      //Reseteo de la IP del nodo destino de la reserva
}

/**
 *
 */
void Noria::SendPacketProcess (Ptr<Packet> packet) {
   double now = Simulator::Now ().GetSeconds ();        //Tiempo de simulacion (en segundos)
   
   /* WifiMac Header */
   WifiMacHeader macHeader;             //Formato de cabecera WifiMAC
   packet->RemoveHeader(macHeader);     //Obtener los datos de la cabecera WifiMAC
   if (!macHeader.IsAck()) {            //Si no es un ACK
   
      /* LLC/SNAP Header */
      LlcSnapHeader llcHeader;                  //Formato de cabecera LLC
      if (packet->PeekHeader(llcHeader) > 0) {  //Si el paquete contiene cabecera LLC/SNAP (Formalismo, pues si lo debe tener)
         packet->RemoveHeader(llcHeader);       //Obtener los datos de la cabecera LLS/SNAP
      }
      
      /* ARP Header */
      ArpHeader arpHeader;                      //Formato de cabecera ARP
      if (packet->PeekHeader(arpHeader) == 0) { //Si el paquete no contiene una cabecera ARP
         
         /* IPv4 Header */
         Ipv4Header ipv4Header;                         //Formato de cabecera IPv4
         if (packet->PeekHeader(ipv4Header) > 0) {      //Si el paquete contiene cabecera IPv4 (Formalismo, pues si lo debe tener)
            packet->RemoveHeader(ipv4Header);           //Obtener los datos de la cabecera IPv4
         }
         
         /* UDP Header */
         UdpHeader udpHeader;
         if (packet->PeekHeader(udpHeader) > 0) {       //Si el paquete contiene cabecera UDP
            
            //Si el puerto UDP destino del paquete no es el mismo del protocolo OLSR
            if (udpHeader.GetDestinationPort() != Commons::OLSR_PORT) { 
               
               // Se almacena el salto al que se envio el paquete
               Mac48Address dstMAC = macHeader.GetAddr1();                              //Destination Address (IEEE 802.11 MAC header)
               Ipv4Address ip = norias.GetRelatedIpAddress (GetIndex(), dstMAC);        //Direccion ip del nodo referenciado
               norias.AddTransmission (GetIndex (), ip, now);                           //--------------------------------------------
               
               // Se almacena el destino y tiempo de transmision del paquete
               Ipv4Address dstIP = ipv4Header.GetDestination(); //Direccion IP destino de la transmision
               norias.AddTransmission (GetIndex(), dstIP, now); //--------------------------------------
            }
         }
      }
   }
}

/**
 *
 */
void Noria::SetRecalculateRoutingTablePeriod (double period) {
   n_recalculateRoutingTablePeriod = period;
   Simulator::Schedule (Seconds (period), &RecalculateTable, n_index);
}

/**
 *
 */
void Noria::ShowConfig () {
   std::cout << "Noria # " << n_index << "" << std::endl                //Escribe el indice del Noria
             << "Direccion MAC: " << GetMacAddress() << "" << std::endl //Escribe la direccion MAC del nodo
             << "Direccion IP: " << GetIpAddress() << "" << std::endl   //Escribe la direccion IP del nodo
             << std::endl;                                              //Fin de linea
}

/**
 *
 */
void Noria::ShowRelations () {
   std::cout << "Noria " << GetIndex() << " identified " << n_relations.GetSize() << " relations" << std::endl;
   n_relations.Show();
   std::cout << std::endl;
}

/**
 *
 */
void Noria::UpdateReservedNodes (double time) {
   n_reservednodes.Update (time);
}

/**
 *
 */
void Noria::UpdateTransmission (Ipv4Address ip, double time) {
   n_transmissions.Update (ip, time);
}

/**
 *
 */
void Noria::WriteConfig () {
   std::ofstream write((filePrefix + Commons::noriaRegistry).c_str(), std::ios::app);
   write << n_index << Commons::csvSymbol 
         << GetMacAddress() << Commons::csvSymbol 
         << GetIpAddress() << std::endl;
   write.close();
}

/**
 *
 */
void Noria::WriteAddressRelations (bool enable) {
   n_writeAddressRelations = enable;
}

/**
 *
 */
void Noria::WriteAddressRelations () {
   std::stringstream name;
   name << "_noria_" << n_index << "_relations.txt";
   std::string fileName = name.str();
   std::ofstream write(fileName.c_str(), std::ios::app);
   write << "Time: " << (Simulator::Now()).GetSeconds() << std::endl 
         << "Noria " << n_index << " detected " << n_relations.GetSize() << " relations:" << std::endl;
   write.close();
   n_relations.Write(fileName);
}

/**
 *
 */
void Noria::WriteReservedNodes (bool enable) {
   n_writeReservedNodes = enable;
}

/**
 *
 */
void Noria::WriteReservedNodes () {
   std::stringstream name;
   name << "_noria_" << n_index << "_reserved.txt";
   std::string fileName = name.str();
   std::ofstream write(fileName.c_str(), std::ios::app);
   write << "Time: " << (Simulator::Now()).GetSeconds() << std::endl 
         << "Noria " << n_index << " identified " << n_reservednodes.GetSize() << " reserved nodes:" << std::endl;
   write.close();
   n_reservednodes.Write (fileName);
}

/**
 *
 */
void Noria::WriteReservationState (bool enable) {
   n_writeReservationState = enable;
}

/**
 *
 */
void Noria::WriteReservationState () {
   std::ofstream write((filePrefix + Commons::stateRegistry).c_str(), std::ios::app);
   write << Simulator::Now().GetSeconds() << Commons::csvSymbol 
         << n_index << Commons::csvSymbol 
         << n_reserved << Commons::csvSymbol 
         << n_reservedTime << Commons::csvSymbol 
         << n_rSrcAddress << Commons::csvSymbol 
         << n_rDstAddress;
   write << std::endl;
   write.close();
}

/**
 *
 */
void Noria::WriteRoutingTable (bool enable) {
   n_writeRoutingTable = enable;
}

/**
 * Write (in file) the OLSR routing table of the node where the Noria is located
 */
void Noria::WriteRoutingTable () {
   std::stringstream name;
   name << "_noria_" << n_index << "_routing.txt";
   std::string fileName = name.str();
   std::ofstream write(fileName.c_str(), std::ios::app);
   write << "Time: " << (Simulator::Now()).GetSeconds() << std::endl;
   write << setw(5) << "Dest" << " | " 
         << setw(5) << "Next" << " | " 
         << setw(5) << "Dist" << " | " 
         << setw(5) << "Intf" << std::endl;
   std::map<Ipv4Address, RoutingTableEntry>::iterator it;
   for (it = n_olsr->m_table.begin(); it != n_olsr->m_table.end(); it++) {
      write << setw(5) << (*it).second.destAddr  << " | " 
            << setw(5) << (*it).second.nextAddr  << " | " 
            << setw(5) << (*it).second.distance  << " | " 
            << setw(5) << (*it).second.interface << std::endl;
   }
   write << std::endl;
   write.close();
}

/**
 *
 */
void Noria::WriteTransmissions (void) {
   n_transmissions.Write ();
}


/**
 * +-------------------------------------------------------------------------+
 * | Clase NoriaContainer                                                    |
 * +-------------------------------------------------------------------------+
 */
 
/**
 *
 */
NoriaContainer::NoriaContainer (void) {
}

/**
 *
 */
void NoriaContainer::Add (Noria noria) {
   m_norias.push_back (noria);
}

/**
 *
 */
void NoriaContainer::AddRelation (uint32_t index, Mac48Address mac, Ipv4Address ip) {
   m_norias[index].AddRelation (mac, ip);
}

/**
 *
 */
void NoriaContainer::AddReserved (uint32_t index, Ipv4Address ip, double time) {
   m_norias[index].AddReserved (ip, time);
}

/**
 *
 */
void NoriaContainer::AddTransmission (uint32_t index, Ipv4Address ip, double time) {
   m_norias[index].AddTransmission (ip, time);
}

/**
 *
 */
void NoriaContainer::Clear (void) {
   m_norias.clear ();
}

/**
 *
 */
void NoriaContainer::EraseReservedNode (uint32_t noriaIndex, uint32_t reservedIndex) {
   m_norias[noriaIndex].EraseReservedNode (reservedIndex);
}

/**
 *
 */
void NoriaContainer::EraseTransmission (uint32_t index, Ipv4Address ip) {
   m_norias[index].EraseTransmission (ip);
}

/**
 *
 */
bool NoriaContainer::ExistTransmission (uint32_t index, Ipv4Address ip) {
   return m_norias[index].ExistTransmission (ip);
}

/**
 *
 */
Noria NoriaContainer::Get (uint32_t i) const {
   return m_norias[i];
}

/**
 * 
 */
uint32_t NoriaContainer::GetNoriaIndex (Mac48Address address) {
   uint32_t i = 0;
   uint32_t nNorias = GetSize();
   bool found = false;
   while (!found && i < nNorias) {
      if (address == m_norias[i].GetMacAddress()) {
         found = true;
      }
      else {
         i += 1;
      }
   }
   return i;
}

/**
 *
 */
double NoriaContainer::GetRecalculateRoutingTablePeriod (uint32_t index) {
   return m_norias[index].GetRecalculateRoutingTablePeriod ();
}

/**
 *
 */
Ipv4Address NoriaContainer::GetRelatedIpAddress (uint32_t index, Mac48Address mac) {
   return m_norias[index].GetRelatedIpAddress (mac);
}

/**
 *
 */
uint32_t NoriaContainer::GetSize (void) const {
   return m_norias.size ();
}

/**
 *
 */
double NoriaContainer::GetTransmissionTime (uint32_t index, Ipv4Address ip) {
   return m_norias[index].GetTransmissionTime (ip);
}

/**
 *
 */
void NoriaContainer::Install (NodeContainer c, Ipv4InterfaceContainer interfaces) {
   uint32_t cont = 0;
   for (NodeContainer::Iterator i = c.Begin (); i != c.End (); ++i) {
      Ipv4Address ipAddress = interfaces.GetAddress (cont);
      Noria agent;
      agent.Install (cont, ipAddress);
      Add (agent);
      cont += 1;
   }
}

/**
 *
 */
bool NoriaContainer::IsReserved (uint32_t index) {
   return m_norias[index].IsReserved ();
}

/**
 *
 */
void NoriaContainer::RecalculateRoutingTable (uint32_t index) {
   m_norias[index].RecalculateRoutingTable ();
}

/**
 *
 */
void NoriaContainer::ReceivePacketProcess (uint32_t index, Ptr<Packet> packet) {
   m_norias[index].ReceivePacketProcess (packet);
}

/**
 *
 */
void NoriaContainer::Reserve (uint32_t index, double now, Ipv4Address srcIP, Ipv4Address dstIP) {
   m_norias[index].Reserve (now, srcIP, dstIP);
}

/**
 *
 */
void NoriaContainer::Reset (uint32_t index) {
   m_norias[index].Reset ();
}

/**
 *
 */
void NoriaContainer::SendPacketProcess (uint32_t index, Ptr<Packet> packet) {
   m_norias[index].SendPacketProcess (packet);
}

/**
 *
 */
void NoriaContainer::SetRecalculateRoutingTablePeriod (double period) {
   uint32_t n = GetSize();
   for (uint32_t i = 0;i < n;i += 1) {
      m_norias[i].SetRecalculateRoutingTablePeriod (period);
   }
}

/**
 *
 */
void NoriaContainer::SetRecalculateRoutingTablePeriod (uint32_t index, double period) {
   m_norias[index].SetRecalculateRoutingTablePeriod (period);
}

/**
 *
 */
void NoriaContainer::ShowConfig () {
   uint32_t nNorias = GetSize ();
   for (uint32_t i = 0;i < nNorias;i += 1) {
      m_norias[i].ShowConfig ();
   }
}

/**
 *
 */
void NoriaContainer::UpdateReservedNodes (uint32_t index, double time) {
   m_norias[index].UpdateReservedNodes (time);
}

/**
 *
 */
void NoriaContainer::UpdateTransmissionTime (uint32_t index, Ipv4Address ip, double time) {
   m_norias[index].UpdateTransmission (ip, time);
}

/**
 *
 */
void NoriaContainer::WriteAddressRelations (bool enable) {
   uint32_t nNorias = GetSize ();
   for (uint32_t i = 0;i < nNorias;i += 1) {
      m_norias[i].WriteAddressRelations (enable);
   }
}

/**
 *
 */
void NoriaContainer::WriteReservationState (bool enable) {
   uint32_t nNorias = GetSize ();
   for (uint32_t i = 0;i < nNorias;i += 1) {
      m_norias[i].WriteReservationState (enable);
   }
}

/**
 *
 */
void NoriaContainer::WriteReservationState (uint32_t index) {
   m_norias[index].WriteReservationState ();
}

/**
 *
 */
void NoriaContainer::WriteReservedNodes (bool enable) {
   uint32_t nNorias = GetSize ();
   for (uint32_t i = 0;i < nNorias;i += 1) {
      m_norias[i].WriteReservedNodes (enable);
   }
}

/**
 *
 */
void NoriaContainer::WriteRoutingTable (uint32_t index) {
   m_norias[index].WriteRoutingTable ();
}

/**
 *
 */
void NoriaContainer::WriteRoutingTables (bool enable) {
   uint32_t nNorias = GetSize ();
   for (uint32_t i = 0;i < nNorias;i += 1) {
      m_norias[i].WriteRoutingTable (enable);
   }
}


/*
* -------[ Clase NoriaSimulation ]----------------->8--------------------
* Definicion de los procedimientos de la clase NoriaSimulation. Esta se 
* encarga de las cuestiones tecnicas de la simulacion (definicion de 
* parametros, topologia, creacion de la red y funcionamiento).
* -----------------------------------------------------------------------
*/

/**
 *
 */
NoriaSimulation::NoriaSimulation () : 
   areaHeight (500.0),                  //Altura (largo) del terreno de simulacion
   areaWidth (500.0),                   //Anchura del terreno de simulacion
   bytesTotal (0),                      //Total de bytes transmitidos
   dataRate ("2048bps"),                //Tasa de datos
   installNorias (true),                // Indica si se instalan los Noria en los nodos
   mTxp (7.5),                          //---------------------------------------
   nNodes (50),                         //Numero de nodos
   nodePause (0),                       //Pausa del movimiento de los nodos (en segundos)
   nodeSpeed (1.5),                     //Velocidad de movimiento de los nodos (m/s)
   nSinks (10),                         //Numero de sinks
   packetsReceived (0),                 //Paquetes recibidos
   phyMode ("DsssRate11Mbps"),          //---------------------------------------
   port (9),                            //Puerto de comunicacion
   recalculatePeriod (2.0),             //Periodo de recalculacion de las tablas de enrutamiento
   totalTime (200.0),                   //---------------------------------------
   traceMobility (false),               //Rastrear movilidad
   writeAddressRelations (false),       //---------------------------------------
   writeAsciiFiles (false),             //---------------------------------------
   writeMobilityFiles (false),          //---------------------------------------
   writePcapFiles (false),              //---------------------------------------
   writeReservationState (false),       //---------------------------------------
   writeReservedNodes (false),          //---------------------------------------
   writeRoutingTables (false)           //---------------------------------------
{
}

/**
 *
 */
void NoriaSimulation::InitFiles () {
   std::ofstream dataOut ((filePrefix + Commons::dataRegistry).c_str ());      //---------------------------------------
   dataOut << "SimulationSecond"  << Commons::csvSymbol         //---------------------------------------
           << "ReceiveRate"       << Commons::csvSymbol         //---------------------------------------
           << "PacketsReceived"   << Commons::csvSymbol         //---------------------------------------
           << "NumberOfSinks"     << Commons::csvSymbol         //---------------------------------------
           << "TransmissionPower" << std::endl;                 //---------------------------------------
   dataOut.close ();                                            //---------------------------------------
   
   std::ofstream noriaOut ((filePrefix + Commons::noriaRegistry).c_str ());    //---------------------------------------
   noriaOut << "index" << Commons::csvSymbol                    //---------------------------------------
            << "MAC"   << Commons::csvSymbol                    //---------------------------------------
            << "IP"    << std::endl;                            //---------------------------------------
   noriaOut.close ();                                           //---------------------------------------
   
   std::ofstream sentOut ((filePrefix + Commons::sentRegistry).c_str (), std::ios::app);       //---------------------------------------
   sentOut << "Time"   << Commons::csvSymbol                                    //---------------------------------------
           << "srcMAC" << Commons::csvSymbol                                    //---------------------------------------
           << "dstMAC" << Commons::csvSymbol                                    //---------------------------------------
           << "srcIP"  << Commons::csvSymbol                                    //---------------------------------------
           << "dstIP"  << std::endl;                                            //---------------------------------------
   sentOut.close ();                                                            //---------------------------------------
   
   std::ofstream receiveOut ((filePrefix + Commons::receivedRegistry).c_str (), std::ios::app);        //---------------------------------------
   receiveOut << "Time"  << Commons::csvSymbol                                          //---------------------------------------
              << "Node"  << Commons::csvSymbol                                          //---------------------------------------
              << "srcIP" << Commons::csvSymbol                                          //---------------------------------------
              << "dstIP" << std::endl;                                                  //---------------------------------------
   receiveOut.close ();                                                                 //---------------------------------------
   
   if (writeReservationState) {
      std::ofstream stateOut ((filePrefix + Commons::stateRegistry).c_str (), std::ios::app);  //---------------------------------------
      stateOut << "Time"         << Commons::csvSymbol                          //---------------------------------------
               << "Noria"        << Commons::csvSymbol                          //---------------------------------------
               << "Reserved"     << Commons::csvSymbol                          //---------------------------------------
               << "ReservedTime" << Commons::csvSymbol                          //---------------------------------------
               << "srcIP"        << Commons::csvSymbol                          //---------------------------------------
               << "dstIP"        << std::endl;                                  //---------------------------------------
      stateOut.close ();                                                        //---------------------------------------
   }
}

/**
 *
 */
void NoriaSimulation::ReceivePacket (Ptr<Socket> socket) {
   Ptr<Packet> packet;
   while (packet = socket->Recv ()) {
      bytesTotal += packet->GetSize ();
      packetsReceived += 1;
      SocketAddressTag tag;
      bool found;
      found = packet->PeekPacketTag (tag);
      if (found) {
         InetSocketAddress addr = InetSocketAddress::ConvertFrom (tag.GetAddress ());
         double now = Simulator::Now ().GetSeconds ();
         uint32_t node = socket->GetNode ()->GetId ();
         
         RegisterPacketReceived (now, node, addr.GetIpv4 ());
         NS_LOG_UNCOND (now << " " << node << " received one packet from " << addr.GetIpv4 ());
         
         //cast addr to void, to suppress 'addr' set but not used.Ccompiler warning in optimized builds
         (void) addr;
      }
      else {
         NS_LOG_UNCOND (Simulator::Now ().GetSeconds () << " " << socket->GetNode ()->GetId () << " received one packet!");
      }
   }
}

/**
 *
 */
void NoriaSimulation::RegisterPacketReceived (double now, uint32_t node, Ipv4Address ip) {
   std::ofstream write ((filePrefix + Commons::receivedRegistry).c_str (), std::ios::app);                     //---------------------------------------
   write << now  << Commons::csvSymbol                                                          //---------------------------------------
         << node << Commons::csvSymbol                                                          //---------------------------------------
         << ip   << Commons::csvSymbol                                                          //---------------------------------------
         << "10.1.1." << (node + 1) << std::endl;                                               //---------------------------------------
   write.close ();                                                                              //---------------------------------------
}

/**
 *
 */
void NoriaSimulation::CheckThroughput () {
   double kbs = (bytesTotal * 8.0) / 1000;                                              //---------------------------------------
   bytesTotal = 0;                                                                      //---------------------------------------
   std::ofstream out ((filePrefix + Commons::dataRegistry).c_str (), std::ios::app);                   //---------------------------------------
   out << Simulator::Now ().GetSeconds () << Commons::csvSymbol                         //Tiempo en segundos
       << kbs << Commons::csvSymbol                                                     //Kilobytes transmitidos
       << packetsReceived << Commons::csvSymbol                                         //Paquetes recibidos
       << nSinks << Commons::csvSymbol                                                  //---------------------------------------
       << mTxp << std::endl;                                                            //---------------------------------------
   out.close ();                                                                        //Cerrar archivo
   packetsReceived = 0;                                                                 //---------------------------------------
   Simulator::Schedule (Seconds (1.0), &NoriaSimulation::CheckThroughput, this);        //---------------------------------------
}

/**
 *
 */
Ptr<Socket> NoriaSimulation::SetupPacketReceive (Ipv4Address addr, Ptr<Node> node) {
   TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");                         //---------------------------------------
   Ptr<Socket> sink = Socket::CreateSocket (node, tid);                                 //---------------------------------------
   InetSocketAddress local = InetSocketAddress (addr, port);                            //---------------------------------------
   sink->Bind (local);                                                                  //---------------------------------------
   sink->SetRecvCallback (MakeCallback (&NoriaSimulation::ReceivePacket, this));        //---------------------------------------
   return sink;                                                                         //---------------------------------------
}

/**
 *
 */
void NoriaSimulation::Run () {

   std::stringstream prefix;                                            // Prefijo de los archivos
   prefix << "_n" << nNodes << ((installNorias) ? "ca" : "sa") << "_";  // Construccion del prefijo de los archivos
   filePrefix = prefix.str();                                           // Asignacion del prefijo de los archivos
   
   InitFiles();         // Inicio de los archivos
   norias.Clear();      // Limpia el contenedor de Norias

   Packet::EnableChecking ();                                                                   //---------------------------------------
   Config::SetDefault ("ns3::OnOffApplication::PacketSize", StringValue ("64"));                //---------------------------------------
   Config::SetDefault ("ns3::OnOffApplication::DataRate", StringValue (dataRate));              //---------------------------------------
   Config::SetDefault ("ns3::WifiRemoteStationManager::NonUnicastMode", StringValue (phyMode)); //---------------------------------------
   
   NodeContainer adHocNodes;    //Contenedor de los nodos de la red
   adHocNodes.Create (nNodes);  //Creacion de los nodos de la red
   
   WifiHelper wifi;                             //Ayudante de la creacion de la red wifi
   wifi.SetStandard (WIFI_PHY_STANDARD_80211b); //Establecimiento del estandar con el que trabajara la red wifi
   
   YansWifiPhyHelper wifiPhy;                   //Ayudante de la creacion el espacio fisico de la red wifi
   wifiPhy = YansWifiPhyHelper::Default ();     //Asignacion de un espacio fisico por defecto
   
   YansWifiChannelHelper wifiChannel;                                           //Ayudante de la creacion del canal de la red wifi
   wifiChannel.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel"); //Establecimiento del modelo de retardo
   wifiChannel.AddPropagationLoss ("ns3::FriisPropagationLossModel");           //Establecimiento del modelo de perdida
   wifiPhy.SetChannel (wifiChannel.Create ());                                  //Asignacion al medio fisico de las caracteristicas del canal
   
   NqosWifiMacHelper wifiMac;                                           //Ayudante de creacion de la capa mac de la red wifi
   wifiMac = NqosWifiMacHelper::Default ();                             //Establecimiento de una capa mac estandar sin QoS
   wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager",        //Establecimiento del gestador de las estaciones remotas. Tasa constante
                                 "DataMode", StringValue (phyMode),     //Modo de datos
                                 "ControlMode", StringValue (phyMode)); //Modo de control
   
   wifiPhy.Set("TxPowerStart", DoubleValue(mTxp));  //------------------------------------------------------
   wifiPhy.Set("TxPowerEnd", DoubleValue(mTxp));    //------------------------------------------------------
   
   wifiMac.SetType ("ns3::AdhocWifiMac");                       //Indicacion que la arquitectura de la red es en modo AdHoc
   NetDeviceContainer adHocDevices;                             //Contenedor de los dispositivos de red de los nodos AdHoc
   adHocDevices = wifi.Install (wifiPhy, wifiMac, adHocNodes);  //Instalacion de los dispositivos de red con en los nodos AdHoc
   
   OlsrHelper olsr;                     //Ayudante de instalacion del protocolo OLSR
   Ipv4ListRoutingHelper list;          //Listado de protocolos de enrutamiento
   list.Add (olsr, 100);                //Agrega el protocolo OLSR a la lista de protocolos de enrutamiento
   InternetStackHelper internet;        //Ayudador de instalacion de los protocolos de internet(IP, TCP, UDP, ...)
   internet.SetRoutingHelper (list);    //Establece la lista de protocolos de enrutamiento en la pila IP
   internet.Install (adHocNodes);       //Instala los protocolos de internet en los nodos AdHoc
   
   Ipv4AddressHelper addressAdhoc;                              //Ayudante de asignacion de direcciones IPv4
   addressAdhoc.SetBase ("10.1.1.0", "255.255.255.0");          //Indica que la red es la 10.1.1.0 con mascara de red 255.255.255.0
   Ipv4InterfaceContainer adHocInterfaces;                      //Contenedor de interfaces de red IPv4
   adHocInterfaces = addressAdhoc.Assign (adHocDevices);        //Asigna las direcciones de red en los dispositivos de red de los nodos AdHoc
   
   ObjectFactory pos;                                                                           //---------------------------------------
   pos.SetTypeId ("ns3::RandomRectanglePositionAllocator");                                     //---------------------------------------
   pos.Set ("X", RandomVariableValue (UniformVariable (0.0, areaWidth)));                       //---------------------------------------
   pos.Set ("Y", RandomVariableValue (UniformVariable (0.0, areaHeight)));                      //---------------------------------------
   Ptr<PositionAllocator> taPositionAlloc = pos.Create ()->GetObject<PositionAllocator> ();     //Creacion del ubicador de posiciones
   
   MobilityHelper mobilityAdhoc;                                                                        //Ayudante de movilidad de los nodos adHoc
   mobilityAdhoc.SetMobilityModel ("ns3::RandomWaypointMobilityModel",                                  //Asignacion del modelo de movilidad
                                   "Speed", RandomVariableValue (UniformVariable (0.0, nodeSpeed)),     //Velocidad de movimiento entre 0.0 y la indicada por parametro
                                   "Pause", RandomVariableValue (ConstantVariable (nodePause)),         //Los nodos no pausan su movimiento (movimiento constante)
                                   "PositionAllocator", PointerValue (taPositionAlloc));                //Ubicador de posiciones
   mobilityAdhoc.SetPositionAllocator (taPositionAlloc);                                                //Asigna al ayudante de movilidad el ubicador de posiciones
   mobilityAdhoc.Install (adHocNodes);                                                                  //Instala en los nodos adHoc la movilidad
   
   if (installNorias) {
      norias.Install (adHocNodes, adHocInterfaces);                                     //Instala los Noria en los nodos indicados
      norias.SetRecalculateRoutingTablePeriod (recalculatePeriod);                      //Indica el tiempo de espera para recalcular las tablas de enrutamiento
      if (writeAddressRelations) norias.WriteAddressRelations (writeAddressRelations);  //Indica si se registran en archivo las relaciones de direcciones
      if (writeReservationState) norias.WriteReservationState (writeReservationState);  //Indica si se registran en archivo los estados de reserva de los nodos
      if (writeReservedNodes) norias.WriteReservedNodes (writeReservedNodes);           //Indica si se registran en archivo los nodos identificados como reservados
      if (writeRoutingTables) norias.WriteRoutingTables (writeRoutingTables);           //Indica si se registran en archivo las tablas de enrutamiento
   }
   
   OnOffHelper onoff1 ("ns3::UdpSocketFactory", Address ());                    //Ayudante para crear aplicaciones con encendido/apagado
   onoff1.SetAttribute ("OnTime", RandomVariableValue (ConstantVariable (1)));  //Especificacion del tiempo de encendido de la aplicacion
   onoff1.SetAttribute ("OffTime", RandomVariableValue (ConstantVariable (0))); //Especificacion del tiempo de apagado de la aplicacion
   
   for (uint32_t i = 0;i < nSinks;i += 1) {
      Ptr<Socket> sink = SetupPacketReceive (adHocInterfaces.GetAddress (i), adHocNodes.Get (i));       //---------------------------------------
      AddressValue remoteAddress (InetSocketAddress (adHocInterfaces.GetAddress (i), port));            //---------------------------------------
      onoff1.SetAttribute ("Remote", remoteAddress);                                                    //---------------------------------------
      UniformVariable var;                                                                              //---------------------------------------
      ApplicationContainer temp = onoff1.Install (adHocNodes.Get (nNodes - i - 1));                     //---------------------------------------
      temp.Start (Seconds (var.GetValue (50.0,51.0)));                                                  //---------------------------------------
      temp.Stop (Seconds (totalTime));                                                                  //---------------------------------------
   }
   
   if (writeMobilityFiles) {
      std::ofstream os;                                 //---------------------------------------
      os.open ((filePrefix + Commons::mobilityRegistry).c_str());       //---------------------------------------
      MobilityHelper::EnableAsciiAll(os);               //---------------------------------------
   }
   
   if (writeAsciiFiles) wifiPhy.EnableAsciiAll (filePrefix + Commons::asciiRegistry);        //---------------------------------------
   if (writePcapFiles) wifiPhy.EnablePcapAll ((filePrefix + Commons::pcapRegistry), true);     //---------------------------------------
   
   CheckThroughput();   //---------------------------------------
      
   FlowMonitorHelper flowmonHelper;             //---------------------------------------
   Ptr<FlowMonitor> flowmon;                    //---------------------------------------
   flowmon = flowmonHelper.InstallAll ();       //---------------------------------------
   flowmon->Start (Seconds (50.0));             //---------------------------------------
   
   Simulator::Stop (Seconds (totalTime));                                       //Indica al simulador al cabo de tanto tiempo debe finalizar
   WriteConfig ();                                                              //---------------------------------------
   Simulator::Run ();                                                           //Inicia la simulacion
   flowmon->SerializeToXmlFile ((filePrefix + Commons::flowRegistry).c_str (), false, false);  //---------------------------------------
   Simulator::Destroy ();                                                       //Una vez finalizada destruye la simulacion
}

/**
 *
 */
void NoriaSimulation::SetParameters (double height, double width, uint32_t nodes, uint32_t sinks, bool norias, double recalculateTime, double time, bool mobility, bool addressRelations, bool asciiFiles, bool mobilityFiles, bool pcapFiles, bool reservationState, bool reservedNodes, bool routingTables) {
   areaHeight = height;                         //------------------------------
   areaWidth = width;                           //------------------------------
   nNodes = nodes;                              //------------------------------
   nSinks = sinks;                              //------------------------------
   installNorias = norias;                      //------------------------------
   recalculatePeriod = recalculateTime;         //------------------------------
   totalTime = time;                            //------------------------------
   traceMobility = mobility;                    //------------------------------
   writeAddressRelations = addressRelations;    //------------------------------
   writeAsciiFiles = asciiFiles;                //------------------------------
   writeMobilityFiles = mobilityFiles;          //------------------------------
   writePcapFiles = pcapFiles;                  //------------------------------
   writeReservationState = reservationState;    //------------------------------
   writeReservedNodes = reservedNodes;          //------------------------------
   writeRoutingTables = routingTables;          //------------------------------
}

/**
 *
 */
void NoriaSimulation::WriteConfig () {
   std::cout << "====================================================" << std::endl;
   std::cout << "Numero de nodos: " << nNodes << std::endl;
   std::cout << "Norias instalados: " << ((installNorias) ? "Si" : "No") << std::endl;
   std::cout << "Prefijo archivos: " << filePrefix << std::endl;
   std::cout << "Dimensiones terreno: " << areaWidth << " x " << areaHeight << std::endl;
   std::cout << "Sinks: " << nSinks << std::endl;
   std::cout << "Tiempo total de simulacion: " << totalTime << " segundos" << std::endl;
   std::cout << "Periodo de recalculacion de tablas de enrutamiento: " << recalculatePeriod << " segundos" << std::endl;
}

/**
 *
 */
int main (int argc, char *argv[]) {

   double areaHeight = 500.0;
   double areaWidth = 500.0;
   uint32_t endNodes = 80;
   uint32_t initNodes = 20;
   uint32_t nodeIncrement = 10;
   uint32_t nSinks = 10;
   uint32_t port = 9;
   double recalculatePeriod = 2.0;
   double totalTime = 200.0;
   bool traceMobility = false;
   bool writeAddressRelations = false;
   bool writeAsciiFiles = false;
   bool writeMobilityFiles = false;
   bool writePcapFiles = false;
   bool writeReservationState = false;
   bool writeReservedNodes = false;
   bool writeRoutingTables = false;

   CommandLine cmd;                                                                                             //---------------------------------------
   cmd.AddValue ("areaHeight", "The height (depth) of the simulation surface", areaHeight);                     //---------------------------------------
   cmd.AddValue ("areaWidth", "The width of the simulation surface", areaWidth);                                //---------------------------------------
   cmd.AddValue ("endNodes", "Numero final de nodos", endNodes);                                                //---------------------------------------
   cmd.AddValue ("initNodes", "Numero inicial de nodos", initNodes);                                            //---------------------------------------
   cmd.AddValue ("nodeIncrement", "Incremento de nodos por simulacion", nodeIncrement);                         //---------------------------------------
   cmd.AddValue ("nSinks", "Number of sinks", nSinks);                                                          //---------------------------------------
   cmd.AddValue ("port", "Communication port", port);                                                           //---------------------------------------
   cmd.AddValue ("recalculatePeriod", "Periodo de tiempo para actualizar las tablas", recalculatePeriod);       //---------------------------------------
   cmd.AddValue ("totalTime", "Tiempo total de la simulacion (en segundos)", totalTime);                        //---------------------------------------
   cmd.AddValue ("traceMobility", "Enable mobility tracing", traceMobility);                                    //---------------------------------------
   cmd.AddValue ("writeAddressRelations", "Write address relations registry files", writeAddressRelations);     //---------------------------------------
   cmd.AddValue ("writeAsciiFiles", "Write packet information in ASCII format", writeAsciiFiles);               //---------------------------------------
   cmd.AddValue ("writeMobilityFiles", "Write mobility information", writeMobilityFiles);                       //---------------------------------------
   cmd.AddValue ("writePcapFiles", "Write packet information in PCAP format", writePcapFiles);                  //---------------------------------------
   cmd.AddValue ("writeReservationState", "Write reservation state registry files", writeReservationState);     //---------------------------------------
   cmd.AddValue ("writeReservedNodes", "Write reserved nodes registry files", writeReservedNodes);              //---------------------------------------
   cmd.AddValue ("writeRoutingTables", "Write routing tables registry files", writeRoutingTables);              //---------------------------------------
   cmd.Parse (argc, argv);                                                                                      //---------------------------------------
   
   for (uint32_t i = initNodes;i <= endNodes;i += nodeIncrement) {
      NoriaSimulation caSimulation;     // Creacion del la simulacion con agentes
      caSimulation.SetParameters (areaHeight, areaWidth, i, nSinks, true, recalculatePeriod, totalTime, traceMobility, writeAddressRelations, writeAsciiFiles, writeMobilityFiles, writePcapFiles, writeReservationState, writeReservedNodes, writeRoutingTables);    // Configuracion de los parametros de simulacion
      caSimulation.Run ();              // Corrida de la simulacion
      
      NoriaSimulation saSimulation;     // Creacion del la simulacion sin agentes
      saSimulation.SetParameters (areaHeight, areaWidth, i, nSinks, false, recalculatePeriod, totalTime, traceMobility, writeAddressRelations, writeAsciiFiles, writeMobilityFiles, writePcapFiles, writeReservationState, writeReservedNodes, writeRoutingTables);    // Configuracion de los parametros de simulacion
      saSimulation.Run ();              // Corrida de la simulacion
   }
   
   return 0;    // Salida correcta del programa
}

