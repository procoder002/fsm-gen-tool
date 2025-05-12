#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <algorithm>

#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xmlschemas.h>

using namespace std;

struct Transition {
    string fromState;
    string action;
    string toState;
};

struct FSM {
    string name;
    string initialState;
    vector<string> states;
    vector<string> actions;
    vector<Transition> transitions;

    void parseXML(const char* filename);
    void printFSM();
    void generateCode();
};

// Helper function to get the text content of an XML node
string getNodeContent(xmlNode* node) {
    if (node && node->children && node->children->content) {
        return (const char*)node->children->content;
    }
    return "";
}

// Function to validate the XML file against the XSD schema
bool validateXML(const char* xmlFile, const char* xsdFile) {
    xmlDoc* doc = xmlReadFile(xmlFile, nullptr, 0);
    if (!doc) {
        cerr << "Error: Could not parse XML file." << endl;
        return false;
    }

    // Load the XSD schema
    xmlSchemaParserCtxt* schemaParserCtxt = xmlSchemaNewParserCtxt(xsdFile);
    if (!schemaParserCtxt) {
        cerr << "Error: Could not create schema parser context." << endl;
        xmlFreeDoc(doc);
        return false;
    }

    xmlSchema* schema = xmlSchemaParse(schemaParserCtxt);
    if (!schema) {
        cerr << "Error: Could not parse schema." << endl;
        xmlSchemaFreeParserCtxt(schemaParserCtxt);
        xmlFreeDoc(doc);
        return false;
    }

    xmlSchemaValidCtxt* schemaValidCtxt = xmlSchemaNewValidCtxt(schema);
    if (!schemaValidCtxt) {
        cerr << "Error: Could not create schema validation context." << endl;
        xmlSchemaFree(schema);
        xmlSchemaFreeParserCtxt(schemaParserCtxt);
        xmlFreeDoc(doc);
        return false;
    }

    // Validate the XML document against the schema
    bool isValid = (xmlSchemaValidateDoc(schemaValidCtxt, doc) == 0);

    if (!isValid) {
        cerr << "Error: XML file does not conform to the schema." << endl;
    }

    // Cleanup
    xmlSchemaFreeValidCtxt(schemaValidCtxt);
    xmlSchemaFree(schema);
    xmlSchemaFreeParserCtxt(schemaParserCtxt);
    xmlFreeDoc(doc);

    return isValid;
}

// Function to parse the XML and populate the FSM structure
void FSM::parseXML(const char* filename) {
    // Parse the XML file
    xmlDoc* doc = xmlReadFile(filename, nullptr, 0);
    if (!doc) {
        cerr << "Error: Could not parse XML file." << endl;
        return;
    }

    // Get the root element <fsm>
    xmlNode* root = xmlDocGetRootElement(doc);
    if (!root) {
        cerr << "Error: Invalid XML format. Root element is not <fsm>." << endl;
        xmlFreeDoc(doc);
        return;
    }

    // Iterate through the child nodes of <fsm>
    for (xmlNode* node = root->children; node; node = node->next) {
        if (node->type != XML_ELEMENT_NODE) continue;

        string nodeName = (const char*)node->name;

        if (nodeName == "name") {
            this->name = getNodeContent(node);
        } else if (nodeName == "initstate") {
            this->initialState = getNodeContent(node);
        } else if (nodeName == "states") {
            // Parse <states>
            for (xmlNode* stateNode = node->children; stateNode; stateNode = stateNode->next) {
                if (stateNode->type == XML_ELEMENT_NODE) {
                    this->states.push_back(getNodeContent(stateNode));
                }
            }
        } else if (nodeName == "actions") {
            // Parse <actions>
            for (xmlNode* actionNode = node->children; actionNode; actionNode = actionNode->next) {
                if (actionNode->type == XML_ELEMENT_NODE) {
                    this->actions.push_back(getNodeContent(actionNode));
                }
            }
        } else if (nodeName == "transitions") {
            // Parse <transitions>
            for (xmlNode* transNode = node->children; transNode; transNode = transNode->next) {
                if (transNode->type == XML_ELEMENT_NODE) {
                    Transition transition;
                    xmlChar* fstate = xmlGetProp(transNode, BAD_CAST "fstate");
                    xmlChar* action = xmlGetProp(transNode, BAD_CAST "action");
                    xmlChar* tstate = xmlGetProp(transNode, BAD_CAST "tstate");

                    if (fstate) transition.fromState = (const char*)fstate;
                    if (action) transition.action = (const char*)action;
                    if (tstate) transition.toState = (const char*)tstate;

                    this->transitions.push_back(transition);

                    // Free the allocated properties
                    xmlFree(fstate);
                    xmlFree(action);
                    xmlFree(tstate);
                }
            }
        }
    }

    // Free the XML document
    xmlFreeDoc(doc);
}

// Function to print the FSM structure
void FSM::printFSM() {
    cout << "FSM Name: " << this->name << endl;
    cout << "Initial State: " << this->initialState << endl;

    cout << "\nStates:" << endl;
    for (const string& state : this->states) {
        cout << "  " << state << endl;
    }

    cout << "\nActions:" << endl;
    for (const string& action : this->actions) {
        cout << "  " << action << endl;
    }

    cout << "\nTransitions:" << endl;
    for (const Transition& trans : this->transitions) {
        cout << "  From: " << trans.fromState << ", Action: " << trans.action << ", To: " << trans.toState << endl;
    }
}

void FSM::generateCode() {
    string hfile = "autogen/"+this->name + ".h";
    string cppfile = "autogen/"+this->name + ".cpp";

    string upperCaseName = this->name;
    transform(upperCaseName.begin(), upperCaseName.end(), upperCaseName.begin(), ::toupper);

     // Generate the header file
    ofstream headerFile(hfile);
    headerFile << "#ifndef " << upperCaseName << "_FSM_GENERATED_H\n";
    headerFile << "#define " << upperCaseName << "_FSM_GENERATED_H\n\n";
    headerFile << "#include <string>\n";
    headerFile << "#include <vector>\n";
    headerFile << "#include \"fsm.h\"\n\n";

    string camelCaseName = this->name;
    toupper(camelCaseName[0]);

    headerFile << "static const std::vector<std::string> stateNames = {";
    for (size_t i = 0; i < this->states.size(); ++i) {
        headerFile << " \"" << this->states[i] << "\"";
        if (i != this->states.size() - 1) headerFile << ",";
    }
    headerFile << "};\n";

    // BaseState class with enum
    headerFile << "class " << camelCaseName << "FSMState {\n";
    headerFile << "  public:\n";
    headerFile << "    enum State {";
    for (size_t i = 0; i < this->states.size(); ++i) {
        headerFile << " " << this->states[i];
        if (i != this->states.size() - 1) headerFile << ",";
    }
    headerFile << "   };\n";

    headerFile << "    " << camelCaseName << "FSMState(State state): _state(state) {}\n";
    headerFile << "    std::string get_name() const { return stateNames[static_cast<int>(_state)]; }\n";
    headerFile << "    int get_val() const { return static_cast<int>(_state); }\n";
    headerFile << "    State get_state() const { return _state; }\n";

    headerFile << "  private:\n";
    headerFile << "    State _state;\n";
    headerFile << "  };\n\n";

    headerFile << "static const std::vector<std::string> actionNames = {";
    for (size_t i = 0; i < this->actions.size(); ++i) {
        headerFile << " \"" << this->actions[i] << "\"";
        if (i != this->actions.size() - 1) headerFile << ",";
    }
    headerFile << "};\n";

    // BaseAction class with enum
    headerFile << "class " << camelCaseName << "FSMAction {\n";
    headerFile << "  public:\n";
    headerFile << "    enum Action {";
    for (size_t i = 0; i < this->actions.size(); ++i) {
        headerFile << " " << this->actions[i];
        if (i != this->actions.size() - 1) headerFile << ",";
    }
    headerFile << "   };\n";

    headerFile << "    " << camelCaseName << "FSMAction(Action action): _action(action) {}\n";
    headerFile << "    std::string get_name() const { return actionNames[static_cast<int>(_action)]; }\n";
    headerFile << "    int get_val() const { return static_cast<int>(_action); }\n";
    headerFile << "    Action get_action() const { return _action; }\n";

    headerFile << "  private:\n";
    headerFile <<  "    Action _action;\n";
    headerFile << "  };\n\n";

    headerFile << "static "<< camelCaseName << "FSMAction::Action createAction(int action) {\n";
    headerFile << "  switch(action) {\n";

    for (size_t i = 0; i < this->actions.size(); ++i) {
        headerFile << "    case " << i << ": return "<< camelCaseName << "FSMAction::" << this->actions[i] << ";\n";
    }
    headerFile << "    default: throw std::invalid_argument(\"Invalid integer value for enum\");\n";
    headerFile << "  };\n";
    headerFile << "}\n";



    // FSMBase class
    headerFile << "class " << camelCaseName << "FSMBase: public FSM {\n";
    headerFile << "public:\n";
    headerFile << "    "<< camelCaseName << "FSMBase(const std::string& id);\n";
    headerFile << "    " << camelCaseName << "FSMState get_fsmstate() const { return _currentState; }\n";
    headerFile << "    bool performAction(" << camelCaseName << "FSMAction action);\n\n";
    headerFile << "    void show_fsm_history(std::stringstream& outstream) const;\n";

    for (size_t i = 0; i < this->transitions.size(); ++i) {
        headerFile << "    virtual void trans_" << this->transitions[i].fromState << "_" << this->transitions[i].action << "_" << this->transitions[i].toState << "();\n";
    }
    
    headerFile << "    struct TargetAction {\n";
    headerFile << "      " << camelCaseName << "FSMState targetState;\n";
    headerFile << "      std::function<void()> callback;\n";
    headerFile << "      TargetAction("<< camelCaseName << "FSMState state, std::function<void()> action) : targetState(state), callback(action) {}\n";
    headerFile << "    };\n";

    headerFile << "    struct Transition {\n";
    headerFile << "      time_t      timestamp;\n";
    headerFile << "      " << camelCaseName << "FSMState    fromState;\n";
    headerFile << "      " << camelCaseName << "FSMAction    action;\n";
    headerFile << "      " << camelCaseName << "FSMState    toState;\n\n";
    headerFile << "      Transition(time_t t, "<< camelCaseName << "FSMState fS, "<< camelCaseName << "FSMAction ac, "<< camelCaseName << "FSMState tS) : timestamp(t), fromState(fS), action(ac), toState(tS) {}\n";
    headerFile << "    };\n";
    headerFile << "private:\n";
    headerFile << "    " << camelCaseName << "FSMState" << " _currentState;\n";
    headerFile << "    std::vector<std::vector<TargetAction>> fsmTransition;\n";
    headerFile << "    std::vector<Transition> fsmHistory;\n";
    headerFile << "};\n\n";

    headerFile << "#endif //" << upperCaseName << "_FSM_GENERATED_H\n";
    headerFile.close();

    // Generate the source file
    ofstream sourceFile(cppfile);
    sourceFile << "#include <iostream>\n";
    sourceFile << "#include \"" << this->name << ".h\"\n\n";
    sourceFile << camelCaseName << "FSMBase::"<< camelCaseName << "FSMBase(const std::string& id): FSM(id), _currentState(" << camelCaseName << "FSMState::" << this->initialState << ") {\n";
    sourceFile << "    // Populating fsm transition object\n";
    sourceFile << "  fsmTransition.resize(" << this->states.size() <<", std::vector<TargetAction>(" << this->actions.size()<<", {" << camelCaseName << "FSMState::" << this->initialState << ", nullptr}));\n";
    for (size_t i = 0; i < this->transitions.size(); ++i) {
        string funcName = "trans_" + this->transitions[i].fromState + "_" + this->transitions[i].action + "_" + this->transitions[i].toState;
        sourceFile << "  fsmTransition[static_cast<int>("<<camelCaseName<<"FSMState::"<<this->transitions[i].fromState<<")][static_cast<int>("
        <<camelCaseName<<"FSMAction::"<<this->transitions[i].action<<")] = TargetAction("<<camelCaseName<<"FSMState::"<<
        this->transitions[i].toState << ", std::bind(&"<<camelCaseName<<"FSMBase::" << funcName << ", this));\n";
    }
    sourceFile << "}\n";
    sourceFile << "bool " << camelCaseName << "FSMBase::performAction(" << camelCaseName << "FSMAction action) {\n";
    sourceFile << "  int currState = _currentState.get_val(), currAction = action.get_val();\n";
    sourceFile << "  if(fsmTransition[currState][currAction].callback == nullptr) {\n";
    sourceFile << "    invalid_transition();\n";
    sourceFile << "    std::cout << get_transition_timestamp(time(NULL)) << \"  [\" << _id << \"] INVALID Transition: \" <<  _currentState.get_name() << \"->\" << action.get_name() << std::endl;\n";
    sourceFile << "    return false;\n";
    sourceFile << "  }\n";

    sourceFile << "  TargetAction ta = fsmTransition[currState][currAction];\n";
    //sourceFile << "  "<<camelCaseName<<"FSMState targetState(target.toState);\n";
    sourceFile << "  time_t now = time(NULL);\n";
    sourceFile << "  std::cout << get_transition_timestamp(now) << \"  [\" << _id << \"] FSM Transition  \" << _currentState.get_name()  << \"->\" << action.get_name() << \"->\" << ta.targetState.get_name() << std::endl;\n";
    sourceFile << "  fsmHistory.push_back({now, _currentState, action, ta.targetState});\n";
    sourceFile << "  if(fsmHistory.size() > 15) fsmHistory.erase(fsmHistory.begin());\n";
    sourceFile << "  _currentState = ta.targetState;\n";
    sourceFile << "  ta.callback();\n";
    sourceFile << "  return true;\n";
    sourceFile << "}\n";


    for (size_t i = 0; i < this->transitions.size(); ++i) {
        sourceFile << "void " << camelCaseName << "FSMBase::trans_" << this->transitions[i].fromState << "_" << this->transitions[i].action << "_" << this->transitions[i].toState << "() {\n";

        sourceFile << "}\n";
    }

    sourceFile << "void "<<camelCaseName<< "FSMBase::show_fsm_history(std::stringstream& outstream) const { \n";
    sourceFile << "  outstream << std::endl <<  _id << \" FSM History (currentTime: \" << get_transition_timestamp(time(NULL)) << \") :- \" << std::endl << std::endl;\n";
    sourceFile << "  for(size_t i = 0; i < fsmHistory.size(); i++) {\n";
    sourceFile << "    outstream << i << \" \" << get_transition_timestamp(fsmHistory[i].timestamp) << \" :: \" << fsmHistory[i].fromState.get_name() << \"->\" << fsmHistory[i].action.get_name() << \"->\" << fsmHistory[i].toState.get_name() << std::endl;\n";
    sourceFile << "  }\n";
    sourceFile << "}\n";

    sourceFile.close();
}

int main(int argc, char* argv[]) {

    if(argc != 2) {
        std::cout << "Usage: fsm_gen <input xml>" << std::endl << std::endl;
        return 1;
    }

   const char* xmlFile = argv[1]; // Replace with your XML file
   const char* xsdFile = "./generator/validation.xml";

    // Initialize libxml2 library
    xmlInitParser();
    FSM fsm;

    // Parse and print the XML file content
    if (validateXML(xmlFile, xsdFile)) {
        fsm.parseXML(xmlFile);
        fsm.printFSM();
        fsm.generateCode();
    } else {
        cerr << "XML validation failed. Exiting." << endl;
    }

    // Clean up libxml2
    xmlCleanupParser();

    return 0;
}
