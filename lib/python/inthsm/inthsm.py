class _HSMAcceptException(Exception):
    pass

class _HSMAbortException(Exception):
    pass

def HHAccept():
    raise _HSMAcceptException

def HHAbort():
    raise _HSMAbortException

class BaseState():
    _guards  = { }
    _actions = { }
    _inits   = { }

    def _guardTrue(self):
        return True

    def _actionNoop(self):
        pass

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

    def init(self, states):
        if self.__subst is None:
            return self.__id
        for st in self.__subst:
            self._inits.get(st, self._actionNoop)()
            states[st].enter(self.__id, states)
            return states[st].init(states)

    def matchTransition(self, e, states):
        trs = self.__trans.get(e, [])
        for tr in trs:
            if self._guards.get(tr[0], self._guardTrue)():
                return tr, self.__id
        if self.__superId != -1:
            return states[self.__superId].matchTransition(e, states)
        return None, -1

    def on(self, e, states):
        tr, st = self.matchTransition(e, states)
        if tr is None:
            return -1
        self.exit(tr[2], states)
        states[st]._actions.get(tr[0], self._actionNoop)()
        states[tr[1]].enter(tr[2], states)
        return states[tr[1]].init(states)

class BaseHSM():
    def __init__(self, cb):
        self.__st = -1
        self.__cb = cb

    def __str__(self):
        return self._name

    def getState(self):
        if self.__st == -1:
            return ""
        return str(self._states[self.__st])

    def run(self, reset=False):
        if reset:
            self.__st = -1
        try:
            if self.__st == -1:
                self._start()
                self._states[self._start_state].enter(-1, self._states)
                self.__st = self._states[self._start_state].init(self._states)

            while True:
                e = self.__cb()
                st = self._states[self.__st].on(e, self._states)
                if st != -1:
                    self.__st = st
                else:
                    pass
        except _HSMAcceptException:
            ec = 0
        except _HSMAbortException:
            ec = -1
        except:
            raise

        return ec
