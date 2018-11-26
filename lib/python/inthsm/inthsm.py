class _HSMAcceptException(Exception):
    pass

class _HSMAbortException(Exception):
    pass

def HHAccept():
    raise _HSMAcceptException

def HHAbort():
    raise _HSMAbortException

class Trace():
    def __init__(self):
        self.__start_trace  = None
        self.__entry_trace  = None
        self.__exit_trace   = None
        self.__init_trace   = None
        self.__action_trace = None

    def __traceStartDefault(self, name):
        print("%s-START; " % name)

    def __traceEntryDefault(self, state):
        print("%s-ENTRY; " % state)

    def __traceExitDefault(self, state):
        print("%s-EXIT; " % state)

    def __traceInitDefault(self, state):
        print("%s-INIT; " % state)

    def __traceActionDefault(self, src, e, dst):
        print("%s-%s-%s; " % (src, e, dst))

    def set(self, start  = None,
                  entry  = None,
                  exit   = None,
                  init   = None,
                  action = None):
        self.__start_trace  = start
        self.__entry_trace  = entry
        self.__exit_trace   = exit
        self.__init_trace   = init
        self.__action_trace = action

    def enableTrace(self):
        self.__start_trace  = self.__traceStartDefault
        self.__entry_trace  = self.__traceEntryDefault
        self.__exit_trace   = self.__traceExitDefault
        self.__init_trace   = self.__traceInitDefault
        self.__action_trace = self.__traceActionDefault

    def disableTrace(self):
        self.set()

    def traceStart(self, name):
        if self.__start_trace is None:
            return
        try:
            self.__start_trace(name)
        except Exception as e:
            print("Tracing START got exception: %s.", e)

    def traceEntry(self, state):
        if self.__entry_trace is None:
            return
        try:
            self.__entry_trace(state)
        except Exception as e:
            print("Tracing ENTRY got exception: %s.", e)

    def traceExit(self, state):
        if self.__exit_trace is None:
            return
        try:
            self.__exit_trace(state)
        except Exception as e:
            print("Tracing EXIT got exception: %s.", e)

    def traceInit(self, state):
        if self.__init_trace is None:
            return
        try:
            self.__init_trace(state)
        except Exception as e:
            print("Tracing INIT got exception: %s.", e)

    def traceAction(self, states, src, e, dst):
        if self.__action_trace is None:
            return
        if dst >= 0:
            s = str(states[dst])
        elif dst == -1:
            s = '-'
        else:
            s = '<'
        try:
            self.__action_trace(str(states[src]), e, s)
        except Exception as e:
            print("Tracing ACTION got exception: %s.", e)

class BaseState():
    _guards  = { }
    _actions = { }
    _inits   = { }

    def _guardTrue(self, pd):
        return True

    def _actionNoop(self, pd):
        pass

    def __init__(self, name, sid, superId, trans, subst=None, trace=None):
        self.__name = name
        self.__id = sid
        self.__superId = superId
        self.__trans = trans
        self.__subst = subst
        self.__trace = trace

    def __str__(self):
        return self.__name

    def _entry(self):
        pass

    def _exit(self):
        pass

    def enter(self, start, states, pd):
        if start != self.__superId:
            states[self.__superId].enter(start, states, pd)
        self.__trace.traceEntry(self.__name)
        self._entry(pd)

    def exit(self, stop, states, pd):
        self.__trace.traceExit(self.__name)
        self._exit(pd)
        if stop != self.__superId:
            states[self.__superId].exit(stop, states, pd)

    def init(self, states, pd):
        if self.__subst is None:
            return self.__id
        for st in self.__subst:
            self.__trace.traceInit(self.__name)
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
            self.__trace.traceAction(states, st, e, tr[1])
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
                self._trace.traceStart(self.__name)
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

    def setTrace(self, start =None,
                       entry =None,
                       exit  =None,
                       init  =None,
                       action=None):
        self._trace.set(start, entry, exit, init, action)
