#include "ElevatorObserver.h"
#include "ECGraphicViewImp.h"
#include "ECElevatorSim.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <string>

int main(int argc, char *argv[])
{

    std::string filename; //get filename from the first command line arguement

    //if command line arguements, then use the filename, else hardcode to test1.txt
    if (argc > 1)
    {
        filename = argv[1];
    }
    else
    {
        filename = "test1.txt";
    }

    std::ifstream inFile(filename); //open file
    if (!inFile.is_open()) //make sure we can open the file
    {
        std::cerr << "Can't open file: " << filename << std::endl;
        return 1;
    }
    

    int numFloors = 0;
    int lenSim = 0;
    
    std::string line;
    while (std::getline(inFile, line))
    {
        if (line.empty() || line[0] == '#') //dont read comments
        {
            continue;
        }
        std::istringstream iss(line);
        if (!(iss >> numFloors >> lenSim))
        {
            std::cerr << "Full arguements not given" << std::endl;
            return 1;
        }
        break;

    }
    std::vector<ECElevatorSimRequest> requests;
    while (std::getline(inFile, line)) //while theres lines
    {
        if (line.empty() || line[0] == '#')
        {
            continue; // skip comment lines
        }
        std::istringstream iss(line);
        int t, src, dest; 
        if (iss >> t >> src >> dest)
        {
            ECElevatorSimRequest req(t, src, dest); //create request and add to vector
            requests.push_back(req);
        }
    }

    inFile.close(); //close file

    //sorting requests by time
    std::sort(requests.begin(), requests.end(), [](const ECElevatorSimRequest& a, const ECElevatorSimRequest& b) {
        return a.GetTime() < b.GetTime();
        });

    //running backend simulation first by itself
    ECElevatorSim sim(numFloors, requests); //create object and send request to backend
    sim.Simulate(lenSim); //simulate using object

    //use new code to get state at each time step
    const std::vector<ECElevatorState>& allStates = sim.GetAllStates();

    //part 2 code: create view
    ECGraphicViewImp view(800, 1100);

    //use new code to feed states to frontend
    ECElevatorObserver elevator(view, numFloors, allStates, lenSim);

    ///same code as part 2
    view.Attach(&elevator);
    
    view.Show();
    

    return 0;
    
}
