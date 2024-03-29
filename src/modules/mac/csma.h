/* -*- mode:c++ -*- ********************************************************
 * file:        csma.h
 *
  * author:     Jerome Rousselot, Marcel Steine, Amre El-Hoiydi,
 *				Marc Loebbers, Yosia Hadisusanto
 *
 * copyright:	(C) 2007-2009 CSEM SA
 * 				(C) 2009 T.U. Eindhoven
 *				(C) 2004 Telecommunication Networks Group (TKN) at
 *              Technische Universitaet Berlin, Germany.
 *
 *              This program is free software; you can redistribute it
 *              and/or modify it under the terms of the GNU General Public
 *              License as published by the Free Software Foundation; either
 *              version 2 of the License, or (at your option) any later
 *              version.
 *              For further information see file COPYING
 *              in the top level directory
 *
 * Funding: This work was partially financed by the European Commission under the
 * Framework 6 IST Project "Wirelessly Accessible Sensor Populations"
 * (WASP) under contract IST-034963.
 ***************************************************************************
 * part of:    Modifications to the MF-2 framework by CSEM
 **************************************************************************/


#ifndef csma_H
#define csma_H


#include <string>
#include <sstream>
#include <vector>
#include <list>

#include "MiXiMDefs.h"
#include "BaseMacLayer.h"
#include "DroppedPacket.h"
/***MOD***/
#include "NicEntry.h"
/*****/
class MacPkt;


/***MOD***/
#include <EnergyConsumption.h>
//

/**
 * @brief Generic CSMA Mac-Layer.
 *
 * Supports constant, linear and exponential backoffs as well as
 * MAC ACKs.
 *
 * @class csma
 * @ingroup csma
 * @ingroup macLayer
 * @author Jerome Rousselot, Amre El-Hoiydi, Marc Loebbers, Yosia Hadisusanto, Andreas Koepke
 * @author Karl Wessel (port for MiXiM)
 *
 * \image html csmaFSM.png "CSMA Mac-Layer - finite state machine"
 */
class MIXIM_API csma : public BaseMacLayer
{
  public:
	csma()
		: BaseMacLayer()
		, nbTxFrames(0)
		, nbRxFrames(0)
		, nbMissedAcks(0)
		, nbRecvdAcks(0)
		, nbDroppedFrames(0)
		, nbTxAcks(0)
		, nbDuplicates(0)
	    , nbDuplicatesComSink1(0)
	    , nbDuplicatesComSink2(0)
		, nbBackoffs(0)
		, backoffValues(0)
		, stats(false)
		, trace(false)
		, backoffTimer(NULL), ccaTimer(NULL), sifsTimer(NULL), rxAckTimer(NULL)
		, macState(IDLE_1)
		, status(STATUS_OK)
		, sifs()
		, macAckWaitDuration()
		, transmissionAttemptInterruptedByRx(false)
		, ccaDetectionTime()
		, rxSetupTime()
		, aTurnaroundTime()
		, macMaxCSMABackoffs(0)
		, macMaxFrameRetries(0)
		, aUnitBackoffPeriod()
		, useMACAcks(false)
		, backoffMethod(CONSTANT)
		, macMinBE(0)
		, macMaxBE(0)
		, initialCW(0)
		, txPower(0)
		, NB(0)
		, macQueue()
		, queueLength(0)
		, txAttempts(0)
		, bitrate(0)
		, droppedPacket()
		, nicId(-1)
		, ackLength(0)
		, ackMessage(NULL)
		, SeqNrParent()
		, SeqNrChild()
	{}

	virtual ~csma();

    /** @brief Initialization of the module and some variables*/
    virtual void initialize(int);

/***MOD***/
    virtual int numInitStages() const {return 5;}
/********/
    /** @brief Delete all dynamically allocated objects of the module*/
    virtual void finish();

    /** @brief Handle messages from lower layer */
    virtual void handleLowerMsg(cMessage*);

    /** @brief Handle messages from upper layer */
    virtual void handleUpperMsg(cMessage*);

    /** @brief Handle self messages such as timers */
    virtual void handleSelfMsg(cMessage*);

    /** @brief Handle control messages from lower layer */
    virtual void handleLowerControl(cMessage *msg);

  protected:
    typedef std::list<MacPkt*> MacQueue;

    /** @name Different tracked statistics.*/
    /*@{*/
	long nbTxFrames;
	long nbRxFrames;
	long nbMissedAcks;
	long nbRecvdAcks;
	long nbDroppedFrames;
	long nbTxAcks;
	long nbDuplicates;
	long nbDuplicatesComSink1;
    long nbDuplicatesComSink2;
	long nbBackoffs;
	double backoffValues;
	/*@}*/

/***MOD***/
	// Added by Jorge
	long nbDroppedFromMACQueueNoTimeInPhase;
	long nbDroppedMACNoTimeBeforePhaseEnd;
/******/

	/** @brief Records general statistics?*/
	bool stats;
	/** @brief Record out put vectors?*/
	bool trace;

    /** @brief MAC states
     * see states diagram.
     */
    enum t_mac_states {
        IDLE_1=1,
        BACKOFF_2,
        CCA_3,
        TRANSMITFRAME_4,
        WAITACK_5,
        WAITSIFS_6,
        TRANSMITACK_7,
		// MOD Just to count the number of states
		NUM_MAC_STATES

    };

    /*************************************************************/
    /****************** TYPES ************************************/
    /*************************************************************/

    /** @brief Kinds for timer messages.*/
    enum t_mac_timer {
      TIMER_NULL=0,
      TIMER_BACKOFF,
      TIMER_CCA,
      TIMER_SIFS,
      TIMER_RX_ACK,
    };

    /** @name Pointer for timer messages.*/
    /*@{*/
    cMessage * backoffTimer, * ccaTimer, * sifsTimer, * rxAckTimer;
    /*@}*/

    /** @brief MAC state machine events.
     * See state diagram.*/
    enum t_mac_event {
      EV_SEND_REQUEST=1,                   // 1, 11, 20, 21, 22
      EV_TIMER_BACKOFF,                    // 2, 7, 14, 15
      EV_FRAME_TRANSMITTED,                // 4, 19
      EV_ACK_RECEIVED,                     // 5
      EV_ACK_TIMEOUT,                      // 12
      EV_FRAME_RECEIVED,                   // 15, 26
      EV_DUPLICATE_RECEIVED,
      EV_TIMER_SIFS,                       // 17
      EV_BROADCAST_RECEIVED, 		   // 23, 24
      EV_TIMER_CCA,
      EV_BEGIN_PHASE, //MOD
      EV_ENERGY_AFTER_CCA
    };

    /** @brief Types for frames sent by the CSMA.*/
    enum t_csma_frame_types {
    	DATA,
    	ACK
    };

    enum t_mac_carrier_sensed {
      CHANNEL_BUSY=1,
      CHANNEL_FREE
    } ;

    enum t_mac_status {
      STATUS_OK=1,
      STATUS_ERROR,
      STATUS_RX_ERROR,
      STATUS_RX_TIMEOUT,
      STATUS_FRAME_TO_PROCESS,
      STATUS_NO_FRAME_TO_PROCESS,
      STATUS_FRAME_TRANSMITTED
    };

    /** @brief The different back-off methods.*/
    enum backoff_methods {
    	/** @brief Constant back-off time.*/
    	CONSTANT = 0,
    	/** @brief Linear increasing back-off time.*/
    	LINEAR,
    	/** @brief Exponentially increasing back-off time.*/
    	EXPONENTIAL,
    };

    /** @brief keep track of MAC state */
    t_mac_states macState;
    t_mac_status status;

    /** @brief Maximum time between a packet and its ACK
     *
     * Usually this is slightly more then the tx-rx turnaround time
     * The channel should stay clear within this period of time.
     */
    simtime_t sifs;

    /** @brief The amount of time the MAC waits for the ACK of a packet.*/
    simtime_t macAckWaitDuration;

    bool transmissionAttemptInterruptedByRx;
    /** @brief CCA detection time */
    simtime_t ccaDetectionTime;
    /** @brief Time to setup radio from sleep to Rx state */
    simtime_t rxSetupTime;
    /** @brief Time to switch radio from Rx to Tx state */
    simtime_t aTurnaroundTime;
/*** Jorge: Time to leave between end of packet and start of the next one according to standard ***/
    simtime_t LIFS;
/***/
    /** @brief maximum number of backoffs before frame drop */
    int macMaxCSMABackoffs;
    /** @brief maximum number of frame retransmissions without ack */
    unsigned int macMaxFrameRetries;
    /** @brief base time unit for calculating backoff durations */
    simtime_t aUnitBackoffPeriod;
    /** @brief Stores if the MAC expects Acks for Unicast packets.*/
    bool useMACAcks;

    /** @brief Defines the backoff method to be used.*/
    backoff_methods backoffMethod;

    /**
     * @brief Minimum backoff exponent.
     * Only used for exponential backoff method.
     */
	int macMinBE;
	/**
	 * @brief Maximum backoff exponent.
     * Only used for exponential backoff method.
     */
	int macMaxBE;

	/** @brief initial contention window size
	 * Only used for linear and constant backoff method.*/
	double initialCW;

    /** @brief The power (in mW) to transmit with.*/
    double txPower;

    /** @brief number of backoff performed until now for current frame */
    int NB;

    /** @brief A queue to store packets from upper layer in case another
    packet is still waiting for transmission..*/
    MacQueue macQueue;

    /** @brief length of the queue*/
    unsigned int queueLength;

    /** @brief count the number of tx attempts
     *
     * This holds the number of transmission attempts for the current frame.
     */
    unsigned int txAttempts;

    /** @brief the bit rate at which we transmit */
    double bitrate;

    /** @brief Inspect reasons for dropped packets */
    DroppedPacket droppedPacket;

    /** @brief publish dropped packets nic wide */
    int nicId;

    /** @brief The bit length of the ACK packet.*/
    int ackLength;
/***MOD***/
    /** @brief This MAC layers MAC address.*/
    //int macaddress;

    //Modified by Victor
    bool macDuplicateFilter;    // Enable de Mac filter for duplicated packets
    bool receptionOnBackoff;    // Enable the reception of packets during Backoff time.
    bool receptionOnCCA;        // Enable the reception of packets during CCA
    bool transmitOnReception;   // Enable a transmission to interrupt a reception
    bool  IsInReception;        // Flag that indicate that the mac is receiving a packet now
    bool checkQueue;            // Flag to indicate that the manageQueue function must be called after a reception
    bool ccaStatusIniIdle;      // Save the status of the channel before to set  the CCA_TIMER
    int ccaSamples;             // Specify the number of times that the mac layer checks the channel during a CCA procedure
    int ccaSamplesCounter;      // Count the number of times that the mac layer had checked the channel during a CCA procedure
    int ccaThreshold;           // How many times should be found the channel busy in order to return a CAF
    float ccaValueBusy;         // Count the number of times that the channel would be found busy in a CCA procedure.
    cMessage * ccaSamplerTimer;  // Msg to set the event that periodically check if the channel is busy in a CCA procedur.
    cMessage * LifsCheckQueue;         //  Introduce a delay to call the manageQueue funktion in order to simulate a LIFS time

    // Modified by Jorge
	simtime_t syncPacketTime;			// Max. duration of a Sync Packet, determines the slot size
	simtime_t fullPhaseTime;			// Duration of the Full Phase or Period
	simtime_t timeComSinkPhase1;			// Duration of every Com Sink Phase 1
    simtime_t timeComSinkPhase2;            // Duration of every Com Sink Phase 2
	simtime_t timeSyncPhase;			// Duration of every Sync Phase, everyone is formed by syncPacketsPerSyncPhase mini sync phases
	simtime_t timeReportPhase;			// Duration of the Report Phase
	simtime_t timeVIPPhase;				// Duration of the VIP Phase
	simtime_t smallTime;				// Time to add to another time when we want to make an event after another when they should execute at the same time
	simtime_t guardTransmitTime;		// Time to leave before the end of the phase together with Backoff + aTurnaroundTime + rxSetupTime to ensure any packet is delivered after the end of the phase
	simtime_t nextPhaseStartTime;		// Time to know the next Phase Start Time
	simtime_t timeFromBackOffToTX;		// Time that goes between the end of the backoff time and the transmission of a packet

	int syncPacketsPerSyncPhase; 		// Determines how many times do we repeat all the slots per sync phase
	double phase2VIPPercentage;			// Percentage of the time Phase Report + Phase VIP that the Phase VIP takes

	NicEntry* computer;					// Pointer to the NIC of the computer to take general data over the configurations
	NicEntry* node;						// Pointer to the NIC of this node

	enum PhaseType{						// Phases of the Full Phase or Period
		SYNC_PHASE_1 = 1,
		REPORT_PHASE ,
		VIP_PHASE,
		SYNC_PHASE_2,
		COM_SINK_PHASE_1,
		SYNC_PHASE_3,
		COM_SINK_PHASE_2
	};

	PhaseType nextPhase;				// To know in which phase are we
	cMessage * beginPhases;				// Event to drop all the elements in the queue at the beginning from every phase

	BaseConnectionManager* cc;			// Pointer to the Propagation Model module

	EnergyConsumption* energy;			// Pointer to the Energy module


/***/
protected:
	// FSM functions
	void fsmError(t_mac_event event, cMessage *msg);
	void executeMac(t_mac_event event, cMessage *msg);
	void updateStatusIdle(t_mac_event event, cMessage *msg);
	void updateStatusBackoff(t_mac_event event, cMessage *msg);
	void updateStatusCCA(t_mac_event event, cMessage *msg);
	void updateStatusTransmitFrame(t_mac_event event, cMessage *msg);
	void updateStatusWaitAck(t_mac_event event, cMessage *msg);
	void updateStatusSIFS(t_mac_event event, cMessage *msg);
	void updateStatusTransmitAck(t_mac_event event, cMessage *msg);
	void updateStatusNotIdle(cMessage *msg);
	void manageQueue();
	void updateMacState(t_mac_states newMacState);

	void attachSignal(MacPkt* mac, simtime_t_cref startTime);
	void manageMissingAck(t_mac_event event, cMessage *msg);
	void startTimer(t_mac_timer timer);

	virtual simtime_t scheduleBackoff();

	virtual cPacket *decapsMsg(MacPkt * macPkt);
	MacPkt * ackMessage;

	//sequence number for sending, map for the general case with more senders
	//also in initialisation phase multiple potential parents
	std::map<LAddress::L2Type, unsigned long> SeqNrParent; //parent -> sequence number

	//sequence numbers for receiving
	std::map<LAddress::L2Type, unsigned long> SeqNrChild; //child -> sequence number

private:
  	/** @brief Copy constructor is not allowed.
  	 */
    csma(const csma&);
  	/** @brief Assignment operator is not allowed.
  	 */
    csma& operator=(const csma&);

};

#endif

