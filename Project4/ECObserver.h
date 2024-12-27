//
//  ECObserver.h
//
//
//  Created by Yufeng Wu on 2/27/20.
//
//

#ifndef ECOBERVER_H
#define ECOBERVER_H

#include <vector>
#include <algorithm>
#include <iostream>

//********************************************
// Observer design pattern: observer interface

class ECObserver
{
public:
    virtual ~ECObserver() {}
    virtual void Update() = 0;
};

//********************************************
// Observer design pattern: subject

class ECObserverSubject
{
public:
    ECObserverSubject() {}
    virtual ~ECObserverSubject() {}
    void Attach(ECObserver* pObs)
    {
        //std::cout << "Adding an observer.\n";
        listObservers.push_back(pObs);
    }
    void Detach(ECObserver* pObs)
    {
        listObservers.erase(std::remove(listObservers.begin(), listObservers.end(), pObs), listObservers.end());
    }
    void Notify()
    {
        //std::cout << "Notify: number of observer: " << listObservers.size() << std::endl;
        for (unsigned int i = 0; i < listObservers.size(); ++i)
        {
            listObservers[i]->Update();
        }
    }

private:
    std::vector<ECObserver*> listObservers;
};


#endif
#pragma once
