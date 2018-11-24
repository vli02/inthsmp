class _HSMAcceptException(Exception):
    pass

class _HSMAbortException(Exception):
    pass

def HHAccept():
    raise _HSMAcceptException

def HHAbort():
    raise _HSMAbortException

def _HHGuardTrue():
    return True

def _HHActionNoop():
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

    def _entry(self):
        pass

    def _exit(self):
        pass

    def enter(self, start, states):
        if start != self.__superId:
            states[self.__superId].enter(start, states)
        self._entry()

    def exit(self, stop, states):
        self._exit()
        if stop != self.__superId:
            states[self.__superId].exit(stop, states)

    def init(self, states, inits):
        if self.__subst is None:
            return self.__id
        for st in self.__subst:
            inits.get(st, _HHActionNoop)()
            states[st].enter(self.__id, states)
            return states[st].init(states, inits)

    def matchTransition(self, e, states, guards):
        trs = self.__trans.get(e, [])
        for tr in trs:
            if guards.get(tr[0], _HHGuardTrue)():
                return tr
        if self.__superId != -1:
            return states[self.__superId].matchTransition(e, states, guards)
        return None

    def on(self, e, states, guards, actions, inits):
        tr = self.matchTransition(e, states, guards)
        if tr is None:
            return -1
        self.exit(tr[2], states)
        actions.get(tr[0], _HHActionNoop)()
        states[tr[1]].enter(tr[2], states)
        return states[tr[1]].init(states, inits)

class BaseHSM():
    def __init__(self, cb):
        self.__st = -1
        self.__cb = cb

    def __str__(self):
        return self._name

    def __eventLoop(self):
        while True:
            e = self.__cb()
            self.__st = self._states[self.__st].on(e, self._states,
                                                      self._guards,
                                                      self._actions,
                                                      self._inits)

    def getState(self):
        if self.__st == -1:
            return ""
        return str(self._states[self.__st])

    def start(self):
        self.__st = -1
        try:
            self._start()
            self._states[self._start_state].enter(-1, self._states)
            self.__st = self._states[self._start_state].init(self._states,
                                                             self._inits)
            self.__eventLoop()
        except _HSMAcceptException:
            ec = 0
        except _HSMAbortException:
            ec = -1
        except:
            raise

        return ec

    def run(self):
        if self.__st == -1:
            ec = self.start()
        else:
            ec = self.__eventLoop()
        return ec
