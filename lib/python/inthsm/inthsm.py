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

    def _guardTrue(self, pd):
        return True

    def _actionNoop(self, pd):
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

    def enter(self, start, states, pd):
        if start != self.__superId:
            states[self.__superId].enter(start, states, pd)
        self._entry(pd)

    def exit(self, stop, states, pd):
        self._exit(pd)
        if stop != self.__superId:
            states[self.__superId].exit(stop, states, pd)

    def init(self, states, pd):
        if self.__subst is None:
            return self.__id
        for st in self.__subst:
            self._inits.get(st, self._actionNoop)(pd)
            states[st].enter(self.__id, states, pd)
            return states[st].init(states, pd)

    def matchTransition(self, e, states, pd):
        trs = self.__trans.get(e, [])
        for tr in trs:
            if self._guards.get(tr[0], self._guardTrue)(pd):
                return tr, self.__id
        if self.__superId != -1:
            return states[self.__superId].matchTransition(e, states, pd)
        return None, -1

    def on(self, e, states, pd):
        tr, st = self.matchTransition(e, states, pd)
        if tr is None:
            return -3
        if tr[1] >= 0:
            self.exit(tr[2], states, pd)
        if tr[1] >= -1:
            states[st]._actions.get(tr[0], self._actionNoop)(pd)
        if tr[1] >= 0:
            states[tr[1]].enter(tr[2], states, pd)
            return states[tr[1]].init(states, pd)
        return tr[1]

class BaseHSM():
    def __init__(self, name, cb, pd):
        self.__name = name
        self.__cb   = cb
        self.__pd   = pd
        self.__st   = -1

    def __str__(self):
        return self.__name

    def getState(self):
        if self.__st == -1:
            return ''
        return str(self._states[self.__st])

    def run(self, reset=False):
        if reset:
            self.__st = -1
        try:
            if self.__st == -1:
                self._start(self.__pd)
                self._states[self._start_state].enter(-1, self._states, self.__pd)
                self.__st = self._states[self._start_state].init(self._states, self.__pd)

            while True:
                e = self.__cb(self.__pd)
                st = self._states[self.__st].on(e, self._states, self.__pd)
                if st >= 0:
                    self.__st = st
                elif st == -1:     # local transition
                    pass
                elif st == -2:     # event deferral
                    pass
                else:              # unhandled event
                    pass
        except _HSMAcceptException:
            ec = 0
        except _HSMAbortException:
            ec = -1
        except:
            raise

        return ec
