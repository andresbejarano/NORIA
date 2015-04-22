/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * NORIA: Node Reservation Intelligent Agent
 * Author: Andres Mauricio Bejarano Posada <abejarano@uninorte.edu.co>
 */

#ifndef NORIA_SIMULATION_H
#define NORIA_SIMULATION_H

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/wifi-module.h"
#include "ns3/applications-module.h"

/*
 * Como se realizo una modificacion en el archivo olsr-routing-protocol.h,
 * no se carga todo el paquete oficial sino que se hace con los archivos
 * principales que se encuentran en la misma carpeta que la simulacion.
 */
#include "olsr-routing-protocol.h"
#include "olsr-helper.h"

using namespace ns3;

/**
 *
 */
class NoriaSimulation {
   
   public:
   
   /**
    * Constructor
    */
   NoriaSimulation (void);
   
   /**
    * Write the headers int the registry .csv files
    */
   void InitFiles (void);
   
   /**
    *
    */
   void RegisterPacketReceived (double now, uint32_t node, Ipv4Address ip);
   
   /**
    * Run the simulation
    */
   void Run (void);
   
   /**
    *
    */
   void SetParameters (double height, double width, uint32_t nodes, uint32_t sinks, bool norias, double recalculateTime, double time, bool mobility, bool addressRelations, bool asciiFiles, bool mobilityFiles, bool pcapFiles, bool reservationState, bool reservedNodes, bool routingTables);
   
   /**
    *
    */
   void WriteConfig (void);
   
   
   private:
   
   /**
    *
    */
   double areaHeight;
   
   /**
    *
    */
   double areaWidth;
   
   /**
    *   Total de bytes enviados
    */
   uint32_t bytesTotal;
   
   /**
    *   Tasa de datos
    */
   std::string dataRate;
   
   /**
    *
    */
   bool installNorias;
   
   /**
    *   Transmission power
    */
   double mTxp;
   
   /**
    *   Numero de nodos que contiene la simulacion
    */
   uint32_t nNodes;
   
   /**
    *
    */
   uint32_t nodePause;
   
   /**
    *
    */
   uint32_t nodeSpeed;
   
   /**
    *   Numero de sinks de la simulacion
    */
   uint32_t nSinks;
   
   /**
    *   Total de paquetes recibidos
    */
   uint32_t packetsReceived;
   
   /**
    *
    */
   std::string phyMode;
   
   /**
    *   Puerto utilizado para el envio de los datos
    */
   uint32_t port;
   
   /**
    *   Indica el tiempo de espera para volver a recalcular la tabla de enrutamiento
    *   (En segundos)
    */
   double recalculatePeriod;
   
   /**
    *   Total simulation time (in seconds)
    */
   double totalTime;
   
   /**
    *   Indica si se rastrea la movilidad de los nodos
    */
   bool traceMobility;
   
   /**
    *
    */
   bool writeAddressRelations;
   
   /**
    * Indicates if the ASCII files must be written
    */
   bool writeAsciiFiles;
   
   /**
    * Indicates if the mobility files must be written
    */
   bool writeMobilityFiles;
   
   /**
    * Indicates if the PCAP files must be written
    */
   bool writePcapFiles;
   
   /**
    *
    */
   bool writeReservationState;
   
   /**
    *
    */
   bool writeReservedNodes;
   
   /**
    *
    */
   bool writeRoutingTables;
   
   /**
    *
    */
   void CheckThroughput();
   
   /**
    *
    */
   void ReceivePacket(Ptr<Socket> socket);
   
   /**
    *
    */
   Ptr<Socket> SetupPacketReceive(Ipv4Address addr, Ptr<Node> node);

};

#endif

