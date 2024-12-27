//
//  ECElevatorSim.cpp
//


#include "ECElevatorSim.h"

using namespace std;

ECElevatorMovement::ECElevatorMovement(EC_ELEVATOR_DIR direc) : direction(EC_ELEVATOR_STOPPED) {} //initialize as stopped by default

ECElevatorMovementUp::ECElevatorMovementUp() : ECElevatorMovement(EC_ELEVATOR_UP) {}
void ECElevatorMovementUp::ChangeDirection(ECElevatorSimRequest& req, EC_ELEVATOR_DIR direc, int& floor, int const currTime) { floor++; } //UP so increace currFloor

ECElevatorMovementDown::ECElevatorMovementDown() : ECElevatorMovement(EC_ELEVATOR_DOWN) {}
void ECElevatorMovementDown::ChangeDirection(ECElevatorSimRequest& req, EC_ELEVATOR_DIR direc, int& floor, int const currTime) { floor--; } //DOWN so decreace currFloor

ECElevatorMovementStop::ECElevatorMovementStop() : ECElevatorMovement(EC_ELEVATOR_STOPPED) {}
void ECElevatorMovementStop::ChangeDirection(ECElevatorSimRequest& req, EC_ELEVATOR_DIR direc, int& floor, int const currTime) //STOP so must determine which direction to go to next
{
    if (!req.IsServiced() && req.IsFloorRequestDone() && req.GetFloorDest() == floor && currTime >= req.GetTime()) //check if passenger inside requested to be dropped on this floor
    {
        req.SetServiced(true); //request done
        req.SetArriveTime(currTime); //user dropped off so update drop off time
    }
    else if (!req.IsServiced() && currTime >= req.GetTime() && req.GetFloorSrc() == floor) //if we are at floor req is on
    {
        req.SetFloorRequestDone(true); //set as picked up user
    }
}

ECElevatorSim::ECElevatorSim(int numFloors, std::vector<ECElevatorSimRequest>& listRequests) : numFloors(numFloors), currFloor(1), currDir(EC_ELEVATOR_STOPPED), requests(listRequests) {} //start at floor 1 and initialize as stopped initially

void ECElevatorSim::Simulate(int lenSim)
{
    for (auto tm = 0; tm < lenSim; tm++) //simulate time
    {
        UpdateDirectionAtTime(tm);

        //ECElevatorSimRequest fakeReq(0, 0, 0);

        //create approproate class object and invoke method to update floor
        if (currDir == EC_ELEVATOR_DOWN)
        {
            ECElevatorMovementDown* down = new ECElevatorMovementDown;
            UpdateElevatorMovement(down, tm);

        }
        else if (currDir == EC_ELEVATOR_UP)
        {
            ECElevatorMovementUp* up = new ECElevatorMovementUp;
            UpdateElevatorMovement(up, tm);
        }
        else //whenever we stop, we must update variables and times for passangers being dropped off or picked up
        {
            ECElevatorMovementStop* stop = new ECElevatorMovementStop;
            for (auto& reqs : requests) //loop through each request and mark each as done if they are done (see Stopped class)
            {
                stop->ChangeDirection(reqs, currDir, currFloor, tm);
            }
        }
        RecordState(tm);
        prevMove = GetCurrDir();
    }
}

//are there any requests in the direction you're currently going?
bool ECElevatorSim::anyDirReqs(EC_ELEVATOR_DIR move, int time) const
{
    for (auto& req : requests)
    {
        if (req.GetTime() <= time && !req.IsServiced())
        {
            if ((move == EC_ELEVATOR_UP && req.GetRequestedFloor() > currFloor) || (move == EC_ELEVATOR_DOWN && req.GetRequestedFloor() < currFloor)) //request is in current direction
            {
                return true;
            }
        }
    }
    return false;
}

//are there any requests on currFloor?
bool ECElevatorSim::anyFloorReq(int const time, int currFloor) const
{
    for (auto& req : requests)
    {
        if (req.GetTime() <= time && req.GetRequestedFloor() == currFloor) { return true; }
    }
    return false;
}

void ECElevatorSim::handleDirectionChangeHelper(ECElevatorSimRequest requestParameter)
{
    if (requestParameter.GetRequestedFloor() < currFloor) { SetCurrDir(EC_ELEVATOR_DOWN); } //go down if needed
    else if (requestParameter.GetRequestedFloor() > currFloor) { SetCurrDir(EC_ELEVATOR_UP); } //go up if needed
    else { SetCurrDir(EC_ELEVATOR_STOPPED); } //else stop
}

void ECElevatorSim::handleDirectionChange(int time)
{
    for (auto& req : requests)
    {
        if (req.GetTime() <= time && !req.IsServiced() && !req.IsFloorRequestDone()) //if user has not been picked up yet
        {
            SetCurrDir(req.GetRequestedFloor() < currFloor ? EC_ELEVATOR_DOWN : EC_ELEVATOR_UP); //pick them up either going up or down                             
        }
        else if (req.GetTime() <= time && !req.IsServiced()) //person picked up but not dropped off yet
        {
            handleDirectionChangeHelper(req);
        }
        else if (req.GetTime() <= time && !req.IsServiced() && req.GetRequestedFloor() == currFloor) //if this request is fully serviced, set as stopped to update later
        {
            SetCurrDir(EC_ELEVATOR_STOPPED);
        }
        prevMove = GetCurrDir(); //keep track of prev move
    }
}

void ECElevatorSim::RecordState(int time)
{
    ECElevatorState state;
    state.floor = currFloor;
    state.dir = currDir;

    for (int i = 0; i < (int)requests.size(); i++) {
        auto& req = requests[i];
        if (req.GetTime() <= time && !req.IsServiced()) {
            int floorWaitOrDest = req.IsFloorRequestDone() ? req.GetFloorDest() : req.GetFloorSrc();
            bool goingUp = req.IsGoingUp();
            RequestInfoAtTime info;
            info.reqIndex = i;
            info.destFloor = req.GetFloorDest();
            info.goingUp = goingUp;

            if (!req.IsFloorRequestDone()) {
                //waiting at req.GetFloorSrc()
                state.waitingMap[floorWaitOrDest].push_back(info);
            }
            else {
                //not serviced yet
                state.onboard.push_back(info);
            }
        }
    }

    recordedStates.push_back(state);
}

void ECElevatorSim::UpdateDirectionAtTime(int tm)
{
    // If there's a request on the current floor at the current time
    if (anyFloorReq(tm, currFloor))
    {
        SetCurrDir(EC_ELEVATOR_STOPPED);
    }
    else if (anyDirReqs(prevMove, tm))
    {
        // Continue moving in the previous direction if requests exist that way
        SetCurrDir(prevMove);
    }
    else
    {
        // Need to handle changing direction based on available requests
        handleDirectionChange(tm);
    }
}

void ECElevatorSim::UpdateElevatorMovement(ECElevatorMovement* movement, int tm)
{
    ECElevatorSimRequest fakeReq(0, 0, 0);
    if (tm < (int)requests.size())
    {
        movement->ChangeDirection(requests[tm], currDir, currFloor, tm);
    }
    else
    {
        movement->ChangeDirection(fakeReq, currDir, currFloor, tm);
    }
}