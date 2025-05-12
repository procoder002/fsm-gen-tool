# fsm-gen-tool

* Provide a XML file with all the FSM states and transitions, this tool will generate C++ code for application to consume and manage the finite state machine in a very intuitive way
* This will generate the base classes, application code can inherit those and override the methods
* Each transition will be logged with timestamp and other details for quick & easy debugging
* On demand reporting of FSM transition history
* Validate the input XML file against a schema

## How to Use:

*  Clone the repo.
```sh
git clone 
```

### Build Requirement:
* g++ compiler (--stdc++11)
* cmake >= 2.8
* libxml2 (sudo apt-get install libxml2-dev libxml2-doc)

### Future Enhancement:


### Reference

* https://www.reddit.com/r/C_Programming/comments/1aw7erf/tools_for_generating_finite_state_machines/
* https://fsmgenerator.sourceforge.net/
* https://www.spiceworks.com/tech/tech-general/articles/what-is-fsm/