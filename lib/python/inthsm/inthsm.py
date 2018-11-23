class HSMAcceptException(Exception):
    pass

class HSMAbortException(Exception):
    pass

def HHAccept():
    raise HSMAcceptException

def HHAbort():
    raise HSMAbortException

def HHGuardTrue():
    return True

def HHActionNoop():
    pass

class BaseState():
    def __init__(self, name, sid, superId, trans, subst=None):
        self.__name = name
        self.__id = sid
        self.__superId = superId
        self.__trans = trans
        self.__subst = subst

    def __str__(self):
        return self.__name

    def enter(self, start, states, entries):
        if start != self.__superId:
            states[self.__superId].enter(start, states, entries)
        entries.get(self.__id, HHActionNoop)()

    def exit(self, stop, states, exits):
        exits.get(self.__id, HHActionNoop)()
        if stop != self.__superId:
            states[self.__superId].exit(stop, states, exits)

    def matchTransition(self, e, states, guards):
        trs = self.__trans.get(e, [[-1, -1, -1]])
        for tr in trs:
            if guards.get(tr[0], HHGuardTrue)():
                return tr
        if self.__superId != -1:
            return states[self.__superId].matchTransition(e, states, guards)
        return None

    def init(self, states, inits, entries):
        if self.__subst is None:
            return self.__id
        for st in self.__subst:
            inits.get(st, HHActionNoop)()
            states[st].enter(self.__id, states, entries)
            return states[st].init(states, inits, entries)

    def on(self, e, states, entries, exits, guards, actions, inits):
        tr = self.matchTransition(e, states, guards)
        if tr is None:
            return -1
        self.exit(tr[2], states, exits)
        actions.get(tr[0], HHActionNoop)()
        states[tr[1]].enter(tr[2], states, entries)
        return states[tr[1]].init(states, inits, entries)
