#ifndef FSM_H
#define FSM_H

#include <string>
#include <ctime>
#include <functional>
#include <bits/stdc++.h>

static std::string get_transition_timestamp(time_t timestamp)
{
    std::tm* ptm = std::localtime(&timestamp);

    char buffer[32];
    // Format: 2023-08-10T10:54:55
    std::strftime(buffer, 32, "%Y-%m-%dT%H:%M:%S", ptm);

    return std::string(buffer);
}

class FSM
{
    public:
        FSM(const std::string& id) : _id(id), _inv_transit(0) {}
        //std::string get_fsmid()const { return _id; }
        void invalid_transition() { ++_inv_transit; }
        unsigned int invalid_transition_count() { return _inv_transit; }
        
    protected:
        std::string     _id;
        unsigned int    _inv_transit;
};

#endif // FSM_H