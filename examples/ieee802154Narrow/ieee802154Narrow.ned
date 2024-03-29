package org.mixim.examples.ieee802154Narrow;

import org.mixim.base.connectionManager.ConnectionManager;
import org.mixim.base.modules.BaseWorldUtility;
import org.mixim.modules.Localization.WorldUtilityStats;
import org.mixim.modules.Localization.PhyMacNode;
import org.mixim.modules.Localization.PhyMacAnchor;
import org.mixim.modules.Localization.PhyMacComputer;

network ieee802154Narrow
{
    parameters:
        double playgroundSizeX @unit(m); // x size of the area the nodes are in (in meters)
        double playgroundSizeY @unit(m); // y size of the area the nodes are in (in meters)
        double playgroundSizeZ @unit(m); // z size of the area the nodes are in (in meters)
        double numComputers; //total number of computers in the network
        double numAnchors; //total number of anchors in the network
        double numNodes; // total number of mobile nodes in the network
        double fullPhaseTime @unit(s) = default(1.5s); // Time of every full phase
        double timeComSinkPhase1 @unit(s) = default(0.5s); // Time of each of the 2 COM_SYNC phases
		double timeComSinkPhase2 @unit(s) = default(0.5s);

        @display("bgb=$playgroundSizeX,$playgroundSizeY,white,,;bgp=10,50");
    submodules:
        world: WorldUtilityStats {
            parameters:
                playgroundSizeX = playgroundSizeX;
                playgroundSizeY = playgroundSizeY;
                playgroundSizeZ = playgroundSizeZ;
                @display("p=129,40;i=misc/globe");

        }
        connectionManager: ConnectionManager {
            parameters:
                @display("p=225,40;b=42,42,rect,yellow;i=abstract/multicast");

        }
        anchor[numAnchors]: PhyMacAnchor {
            parameters:
                numAnchors = numAnchors;
                @display("p=231,147;b=42,42,rect,yellow;i=device/accesspoint");

        }
        node[numNodes]: PhyMacNode {
            parameters:
                numNodes = numNodes;
                @display("p=60,147;b=0,0,rect,yellow;i=device/pocketpc");

        }
        computer[numComputers]: PhyMacComputer {
            parameters:
                numComputers = numComputers;
                @display("p=141,118;b=50,50,rect,red;i=device/laptop");

        }
    connections allowunconnected:
                         // all connections and gates are to be generated dynamically

}

