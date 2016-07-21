keypress sample code:
=====================
1. keypress.def
        A sample state machine implementation as shown on diagram keypress.png, to demonstrate
        - Automatically define all event and state IDs;
        - An entry and exit function are explicitly defined for each state;
        - A start transition function is explicitly defined;
        - All initial transitions are explicitly specified;
        - All transition actions are explicitly specified;
        - All actions are to print the action description;
        - Demo of internal and external transitions including self transition.

2. keypress2.def
        The same state machine as on diagram keypress.png, but additinally demonstrate:
        - Use manually defined event and state IDs;
        - Use the macro interfaces provided by generated code to print all action information.

3. keypress3.def
        State machine sample as on diagram keypress3.png which is similar with previous one, but
        additionally demonstrate:
        - Use local transitions;
        - Use event wildchar;
        - Use event deferral;
        # - Use event queues.
        - Use lock and unlock functions;
        - Use run to history.