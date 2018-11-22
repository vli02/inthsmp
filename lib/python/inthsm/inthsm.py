class HSMAcceptException(Exception):
    pass

class HSMAbortException(Exception):
    pass

def HHAccept():
    raise HSMAcceptException

def HHAbort():
    raise HSMAbortException

def __HHGuardFalse():
    return False

def __HHActionNoop():
    pass

class BaseState():
    def __init__(self, name, sid, superId, trans={}, initTrans=None):
        self.__name = name
        self.__id = sid
        self.__superId = superId
        self.__trans = trans
        self.__initTrans = initTrans

    def __str__(self):
        return self.__name

    def enter(self, start):
        if start != self.__superId:
            __hh_states[self.__superId].enter(start)
        __hh_entries.get(self.__id, __HHActionNoop)()

    def exit(self, stop):
        __hh_exits.get(self.__id, __HHActionNoop)()
        if stop != self.__superId:
            __hh_states[self.__superId].exit(stop)

    def matchTransition(self, e):
        trs = self.__trans.get(e, [[-1, -1, -1]])
        for tr in trs:
            if __hh_guards.get(tr[0], __HHGuardFalse)():
                return tr
        if self.__superId != -1:
            return __hh_states[self.__superId].matchTransition(e)
        return None

    def init(self):
        if self.__initTrans is None:
            return self.__id
        for tr in self.__initTrans:
            __hh_inits.get(tr[0], __HHActionNoop)()
            __hh_states[tr[1]].enter(self.__id)
            return __hh_states[tr[1]].init()

    def on(self, e):
        tr = self.matchTransition(e)
        if tr is None:
            return -1
        self.exit(tr[2])
        __hh_actions.get(tr[0], __HHActionNoop)()
        __hh_states[tr[1]].enter(tr[2])
        return __hh_states[tr[1]].init()
