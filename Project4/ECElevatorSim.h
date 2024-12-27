//
//  ECElevatorSim.h
//  
//
//  Created by Yufeng Wu on 6/27/23.
//  Elevator simulation

#ifndef ECElevatorSim_h
#define ECElevatorSim_h

#include "ECObserver.h"
#include "ECGraphicViewImp.h"
#include "ECElevatorSim.h"


#include <iostream>
#include <set>
#include <vector>
#include <map>
#include <string>

//*****************************************************************************
// DON'T CHANGE THIS CLASS
// 
// Elevator simulation request: 
// (i) time: when the request is made
// (ii) floorSrc: which floor the user is at at present
// (iii) floorDest floor: where the user wants to go; we assume floorDest != floorSrc
// 
// Note: a request is in three stages:
// (i) floor request: the passenger is waiting at floorSrc; once the elevator arrived 
// at the floor (and in the right direction), move to the next stage
// (ii) inside request: passenger now requests to go to a specific floor once inside the elevator
// (iii) Once the passenger arrives at the floor, this request is considered to be "serviced"
//
// two sspecial requests:
// (a) maintenance start: floorSrc=floorDest=-1; put elevator into maintenance 
// starting at the specified time; elevator starts at the current floor
// (b) maintenance end: floorSrc=floorDest=0; put elevator back to operation (from the current floor)

class ECElevatorSimRequest
{
public:
    ECElevatorSimRequest(int timeIn, int floorSrcIn, int floorDestIn) : time(timeIn), floorSrc(floorSrcIn), floorDest(floorDestIn), fFloorReqDone(false), fServiced(false), timeArrive(-1) {}
    ECElevatorSimRequest(const ECElevatorSimRequest& rhs) : time(rhs.time), floorSrc(rhs.floorSrc), floorDest(rhs.floorDest), fFloorReqDone(rhs.fFloorReqDone), fServiced(rhs.fServiced), timeArrive(rhs.timeArrive) {}
    int GetTime() const { return time; } //time of request
    int GetFloorSrc() const { return floorSrc; } //where user waiting
    int GetFloorDest() const { return floorDest; } //where user wants to go
    bool IsGoingUp() const { return floorDest >= floorSrc; } //is user going up

    // Is this passenger in the elevator or not
    bool IsFloorRequestDone() const { return fFloorReqDone; } //has user boarded elevator?
    void SetFloorRequestDone(bool f) { fFloorReqDone = f; } //set as user having boarded elevator

    // Is this event serviced (i.e., the passenger has arrived at the desstination)?
    bool IsServiced() const { return fServiced; } //has user reacher their destination floor?
    void SetServiced(bool f) { fServiced = f; } //set as user having reached destination floor

    // Get the floor to service
    // If this is in stage (i): waiting at a floor, return that floor waiting at
    // If this is in stage (ii): inside an elevator, return the floor going to
    // Otherwise, return -1
    int GetRequestedFloor() const {
        if (IsServiced()) {
            return -1;
        }
        else if (IsFloorRequestDone()) {
            return GetFloorDest();
        }
        else {
            return GetFloorSrc();
        }
    }

    // Wait time: get/set. Note: you need to maintain the wait time yourself!
    int GetArriveTime() const { return timeArrive; } //when user reached destination floor
    void SetArriveTime(int t) { timeArrive = t; } //set when user reached destination floor

    // Check if this is the special maintenance start request
    bool IsMaintenanceStart() const { return floorSrc == -1 && floorDest == -1; }
    bool IsMaintenanceEnd() const { return floorSrc == 0 && floorDest == 0; }

private:
    int time;           // time of request made
    int floorSrc;       // which floor the request is made
    int floorDest;      // which floor is going
    bool fFloorReqDone;   // is this passenger passing stage one (no longer waiting at the floor) or not
    bool fServiced;     // is this request serviced already?
    int timeArrive;     // when the user gets to the desitnation floor
};

//*****************************************************************************
// Elevator moving direction

typedef enum
{
    EC_ELEVATOR_STOPPED = 0,    // not moving
    EC_ELEVATOR_UP,             // moving up
    EC_ELEVATOR_DOWN            // moving down
} EC_ELEVATOR_DIR;

//*****************************************************************************
// Add your own classes here...

//holds info about where passenger wants to go
struct RequestInfoAtTime
{
    int reqIndex;
    int destFloor;
    bool goingUp;
};

struct ECElevatorState
{
    int floor;
    EC_ELEVATOR_DIR dir;
    std::map<int, std::vector<RequestInfoAtTime>> waitingMap; //map since floor and num ppl
    std::vector<RequestInfoAtTime> onboard; //plain vector for ppl in cabin
};

class ECElevatorMovement
{
public:
    ECElevatorMovement(EC_ELEVATOR_DIR direc); //default constructoir
    virtual void ChangeDirection(ECElevatorSimRequest& req, EC_ELEVATOR_DIR direc, int& floor, int const currTime) = 0; //virtual method
    virtual ~ECElevatorMovement() {} //destreuctor
private:
    EC_ELEVATOR_DIR direction; //to save direc
};

class ECElevatorMovementUp : public ECElevatorMovement //UP movement class inherits
{
public:
    ECElevatorMovementUp();
    virtual void ChangeDirection(ECElevatorSimRequest& req, EC_ELEVATOR_DIR direc, int& floor, int const currTime);
};

class ECElevatorMovementDown : public ECElevatorMovement //DOWN movement class inherits
{
public:
    ECElevatorMovementDown();
    virtual void ChangeDirection(ECElevatorSimRequest& req, EC_ELEVATOR_DIR direc, int& floor, int const currTime);
};

class ECElevatorMovementStop : public ECElevatorMovement //STOP movement class inherits
{
public:
    ECElevatorMovementStop();
    virtual void ChangeDirection(ECElevatorSimRequest& req, EC_ELEVATOR_DIR direc, int& floor, int const currTime);
};

//*****************************************************************************
// Simulation of elevator

class ECElevatorSim
{
public:
    // numFloors: number of floors serviced (floors numbers from 1 to numFloors)
    ECElevatorSim(int numFloors, std::vector<ECElevatorSimRequest>& listRequests);

    // free buffer
    ~ECElevatorSim() {}

    // Simulate by going through all requests up to certain period of time (as specified in lenSim)
    // starting from time 0. For example, if lenSim = 10, simulation stops at time 10 (i.e., time 0 to 9)
    // Caution: the list of requests contain all requests made at different time;
    // at a specific time of simulation, some events may be made in the future (which you shouldn't consider these future requests)
    void Simulate(int lenSim);

    // The following methods are about querying/setting states of the elevator
    // which include (i) number of floors of the elevator, 
    // (ii) the current floor: which is the elevator at right now (at the time of this querying). Note: we don't model the tranisent states like when the elevator is between two floors
    // (iii) the direction of the elevator: up/down/not moving

    // Get num of floors
    int GetNumFloors() const { return numFloors; }

    int GetCurrFloor() const { return currFloor; } // Get current floor
    void SetCurrFloor(int f) { currFloor = f; } // Set current floor

    EC_ELEVATOR_DIR GetCurrDir() const { return currDir; } // Get current direction
    void SetCurrDir(EC_ELEVATOR_DIR dir) { currDir = dir; } // Set current direction

    const std::vector<ECElevatorState>& GetallStates() const { return recordedStates; }

    //my methods
    bool anyFloorReq(int const time, int currFloor) const;
    bool anyDirReqs(EC_ELEVATOR_DIR move, int time) const;
    void handleDirectionChange(int time);
    void handleDirectionChangeHelper(ECElevatorSimRequest requestParameter);
    const std::vector<ECElevatorState>& GetAllStates() const { return recordedStates; }

private:
    // Your code here
    std::vector<ECElevatorSimRequest>& requests;
    int currFloor;
    int numFloors;
    EC_ELEVATOR_DIR currDir;
    EC_ELEVATOR_DIR prevMove = EC_ELEVATOR_STOPPED;

    std::vector<ECElevatorState> recordedStates;

    void RecordState(int time);

    void UpdateDirectionAtTime(int tm);
    void UpdateElevatorMovement(ECElevatorMovement* movement, int tm);
};


#endif /* ECElevatorSim_h */