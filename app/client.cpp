#include <iostream>
#include <unistd.h>
#include "process_state.h"
using namespace std;

#define SECONDS_IN_A_MINUTE     60
#define SECONDS_IN_A_HOUR       3600    // 60 * 60
#define SECONDS_IN_A_DAY        86400   // 24 * 60 * 60

std::string convert_human_time(uint64_t diff_time)
{
    std::stringstream human_time;

    if(diff_time < 0) return "0s";

    if(diff_time >= SECONDS_IN_A_DAY)
    {
        human_time << (diff_time/SECONDS_IN_A_DAY) << "d " ;
        diff_time = diff_time % SECONDS_IN_A_DAY;
    }

    if(diff_time >= SECONDS_IN_A_HOUR)
    {
        human_time << (diff_time/SECONDS_IN_A_HOUR) << "h " ;
        diff_time = diff_time % SECONDS_IN_A_HOUR;
    }

    if(diff_time >= SECONDS_IN_A_MINUTE)
    {
        human_time << (diff_time/SECONDS_IN_A_MINUTE) << "m " ;
        diff_time = diff_time % SECONDS_IN_A_MINUTE;
    }

    human_time << diff_time << "s";

    return human_time.str();
}

std::string calculate_diff_time(uint64_t start_time, uint64_t end_time)
{
    uint64_t diff_time = end_time - start_time;

    return convert_human_time(diff_time);
}

class app1State : public process_stateFSMBase {
  public:
    app1State(const string& id): process_stateFSMBase(id) {}
    void trans_NEW_ADMITTED_READY();
    void trans_READY_SCHEDULAR_DISPATCH_RUNNING();
    void trans_RUNNING_INTERRUPT_READY();
    void trans_RUNNING_EXIT_TERMINATED();
    void trans_RUNNING_IO_WAIT_WAITING();

    string process_uptime() { return get_fsmstate().get_state() == process_stateFSMState::TERMINATED ?  calculate_diff_time(spawn_time, exit_time) : calculate_diff_time(spawn_time, time(NULL)); }
    string process_running_time() { return convert_human_time(running_time); }
  private:
    uint64_t running_time = 0;
    time_t running_state_entry_time;
    //int waiting_time;
    time_t spawn_time;
    time_t exit_time;
};

void app1State::trans_NEW_ADMITTED_READY() {
    spawn_time = time(NULL);
    // Add custom logic as per application needs
    // Send notification, calculate Uptime etc.
}

void app1State::trans_READY_SCHEDULAR_DISPATCH_RUNNING() {
    cout << "app1 trans_READY_SCHEDULAR_DISPATCH_RUNNING" << endl;
    running_state_entry_time = time(NULL);
}

void app1State::trans_RUNNING_INTERRUPT_READY() {
    time_t curr_time = time(NULL);
    running_time += curr_time - running_state_entry_time;
}

void app1State::trans_RUNNING_IO_WAIT_WAITING() {
    time_t curr_time = time(NULL);
    running_time += curr_time - running_state_entry_time;
}

void app1State::trans_RUNNING_EXIT_TERMINATED() {
    exit_time = time(NULL);
}

class app2State : public process_stateFSMBase {
  public:
    app2State(const string& id): process_stateFSMBase(id) {}
    void trans_RUNNING_INTERRUPT_READY();
    void trans_WAITING_IO_COMPLETION_READY();
};

void app2State::trans_RUNNING_INTERRUPT_READY() {
    cout << "app2 trans_RUNNING_INTERRUPT_READY" << endl;
}

void app2State::trans_WAITING_IO_COMPLETION_READY() {
    cout << "app2 trans_WAITING_IO_COMPLETION_READY" << endl;
}

/*
We can create multiple FSM similar way, each of then will work/move independently
app2State app2_state("app2");
app2_state.performAction(process_stateFSMAction::ADMITTED);
*/

int main() {
    srand(time(NULL));
    app1State app1_state("app1");
    app1_state.performAction(process_stateFSMAction::ADMITTED);

    int itr = 10;
    while(itr--) {
        sleep(rand()%5);
        app1_state.performAction(createAction(rand()%5));
    }

    std::stringstream outstream;
    app1_state.show_fsm_history(outstream);
    cout << outstream.str() << endl;

    cout << "Uptime: " << app1_state.process_uptime() << " , Running time: " << app1_state.process_running_time() << endl;
    cout << "Invalid transition count : " << app1_state.invalid_transition_count() << endl;
}
