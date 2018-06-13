using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace TQuant.Stralet
{
    #region States
    public sealed class CurrentState<TS>
    {
        public CurrentState(TS state)
        {
            State = state;
        }

        public TS State { get; }

        #region Equality
        private bool Equals(CurrentState<TS> other)
        {
            return EqualityComparer<TS>.Default.Equals(State, other.State);
        }

        public override bool Equals(object obj)
        {
            if (ReferenceEquals(null, obj)) return false;
            if (ReferenceEquals(this, obj)) return true;
            return obj is CurrentState<TS> && Equals((CurrentState<TS>)obj);
        }

        public override int GetHashCode()
        {
            unchecked
            {
                return EqualityComparer<TS>.Default.GetHashCode(State);
            }
        }
        public override string ToString()
        {
            return $"CurrentState <{State}>";
        }
        #endregion
    }

    public sealed class Transition<TS>
    {
        public Transition(TS from, TS to)
        {
            To = to;
            From = from;
        }

        public TS From { get; }
        public TS To { get; }

        #region Equality
        private bool Equals(Transition<TS> other)
        {
            return EqualityComparer<TS>.Default.Equals(From, other.From) && EqualityComparer<TS>.Default.Equals(To, other.To);
        }

        /// <inheritdoc/>
        public override bool Equals(object obj)
        {
            if (ReferenceEquals(null, obj)) return false;
            if (ReferenceEquals(this, obj)) return true;
            return obj is Transition<TS> && Equals((Transition<TS>)obj);
        }

        public override int GetHashCode()
        {
            unchecked
            {
                int hashCode = EqualityComparer<TS>.Default.GetHashCode(From);
                hashCode = (hashCode * 397) ^ EqualityComparer<TS>.Default.GetHashCode(To);
                return hashCode;
            }
        }

        /// <inheritdoc/>
        public override string ToString()
        {
            return $"Transition({From}, {To})";
        }

        #endregion
    }

    /// <summary>
    /// Reason why this <see cref="FSM{T,S}"/> is shutting down
    /// </summary>
    public abstract class Reason { }

    /// <summary>
    /// Default <see cref="Reason"/> if calling Stop().
    /// </summary>
    public sealed class Normal : Reason
    {
#pragma warning disable 618
        public static Normal Instance { get; } = new Normal();
#pragma warning restore 618
    }

    //        /// <summary>
    //        /// Reason given when someone as calling <see cref="FSM{TState,TData}.Stop()"/> from outside;
    //        /// also applies to <see cref="ActorSystem"/> supervision directive.
    //        /// </summary>
    //        public sealed class Shutdown : Reason
    //        {
    //#pragma warning disable 618
    //            public static Shutdown Instance { get; } = new Shutdown();
    //#pragma warning restore 618
    //        }

    /// <summary>
    /// Signifies that the <see cref="FSM{T,S}"/> is shutting itself down because of an error,
    /// e.g. if the state to transition into does not exist. You can use this to communicate a more
    /// precise cause to the <see cref="FSM{T,S}.OnTermination"/> block.
    /// </summary>
    public sealed class Failure : Reason
    {
        public Failure(object cause)
        {
            Cause = cause;
        }

        public object Cause { get; }

        public override string ToString() => $"Failure({Cause})";
    }

    /// <summary>
    /// Used in the event of a timeout between transitions
    /// </summary>
    public sealed class StateTimeout
    {
#pragma warning disable 618
        public static StateTimeout Instance { get; } = new StateTimeout();
#pragma warning restore 618
    }

    public class State<TS, TD> : IEquatable<State<TS, TD>>
    {
        public State(TS stateName, TD stateData, TimeSpan? timeout = null, Reason stopReason = null)//, IReadOnlyList<object> replies = null, bool notifies = true)
        {
            StopReason = stopReason;
            Timeout = timeout;
            StateData = stateData;
            StateName = stateName;
        }

        public TS StateName { get; }

        public TD StateData { get; }

        public TimeSpan? Timeout { get; }

        public Reason StopReason { get; }

        internal State<TS, TD> Copy(TimeSpan? timeout, Reason stopReason = null)//, IReadOnlyList<object> replies = null)
        {
            return new State<TS, TD>(StateName, StateData, timeout, stopReason ?? StopReason);//, replies ?? Replies, Notifies);
        }

        /// <summary>
        /// Modify the state transition descriptor to include a state timeout for the 
        /// next state. This timeout overrides any default timeout set for the next state.
        /// <remarks>Use <see cref="TimeSpan.MaxValue"/> to cancel a timeout.</remarks>
        /// </summary>
        public State<TS, TD> ForMax(TimeSpan timeout)
        {
            if (timeout <= TimeSpan.MaxValue)
                return Copy(timeout);
            return Copy(timeout: null);
        }

        public State<TS, TD> Using(TD nextStateData)
        {
            return new State<TS, TD>(StateName, nextStateData, Timeout, StopReason);//, Replies, Notifies);
        }

        internal State<TS, TD> WithStopReason(Reason reason)
        {
            return Copy(Timeout, reason);
        }

        public override string ToString()
        {
            return $"{StateName}, {StateData}";
        }

        public bool Equals(State<TS, TD> other)
        {
            if (ReferenceEquals(null, other)) return false;
            if (ReferenceEquals(this, other)) return true;
            return EqualityComparer<TS>.Default.Equals(StateName, other.StateName) &&
                   EqualityComparer<TD>.Default.Equals(StateData, other.StateData) &&
                   Timeout.Equals(other.Timeout) && Equals(StopReason, other.StopReason);
        }

        public override bool Equals(object obj)
        {
            if (ReferenceEquals(null, obj)) return false;
            if (ReferenceEquals(this, obj)) return true;
            if (obj.GetType() != GetType()) return false;
            return Equals((State<TS, TD>)obj);
        }

        public override int GetHashCode()
        {
            unchecked
            {
                var hashCode = EqualityComparer<TS>.Default.GetHashCode(StateName);
                hashCode = (hashCode * 397) ^ EqualityComparer<TD>.Default.GetHashCode(StateData);
                hashCode = (hashCode * 397) ^ Timeout.GetHashCode();
                hashCode = (hashCode * 397) ^ (StopReason != null ? StopReason.GetHashCode() : 0);
                return hashCode;
            }
        }
    }

    public sealed class Event<TD>
    {
        public Event(object fsmEvent, TD stateData)
        {
            StateData = stateData;
            FsmEvent = fsmEvent;
        }

        public object FsmEvent { get; }

        public TD StateData { get; }

        public override string ToString()
        {
            return $"Event: <{FsmEvent}>, StateData: <{StateData}>";
        }
    }

    /// <summary>
    /// Class representing the state of the <see cref="FSM{TS,TD}"/> within the OnTermination block.
    /// </summary>
    public sealed class StopEvent<TS, TD>
    {
        public StopEvent(Reason reason, TS terminatedState, TD stateData)
        {
            StateData = stateData;
            TerminatedState = terminatedState;
            Reason = reason;
        }

        public Reason Reason { get; }

        public TS TerminatedState { get; }

        public TD StateData { get; }

        public override string ToString()
        {
            return $"Reason: <{Reason}>, TerminatedState: <{TerminatedState}>, StateData: <{StateData}>";
        }
    }

    public interface IFsmContext
    {
        bool IsTimerActive(string name);
        void ClearTimers();
        void SetTimer (string name, object data, TimeSpan timeout, bool repeat);
        void KillTimer(string name);
        void PostEvent(object msg);
        void Stop(Reason reason);
    }

    #endregion
    public class BaseFsm<TState, TData>
    {
        private IFsmContext fsm_context;

        public BaseFsm()
        {
        }

        public void SetFsmContext(IFsmContext context)
        {
            this.fsm_context = context;
        }

        public class OnTimer
        {
            public string Name { get; }
            public object Data { get; }

            public OnTimer(string name, object data)
            {
                this.Name = name;
                this.Data = data;
            }
        }

        public class IllegalStateException : Exception
        {
            public IllegalStateException(string message) : base(message)//, innerException)
            {
            }
        }

        public delegate State<TState, TData> StateFunction(Event<TData> fsmEvent);

        public delegate void TransitionHandler(TState initialState, TState nextState);

        public void When(TState stateName, StateFunction func, TimeSpan? timeout = null)
        {
            Register(stateName, func, timeout);
        }

        public void StartWith(TState stateName, TData stateData, TimeSpan? timeout = null)
        {
            _currentState = new State<TState, TData>(stateName, stateData, timeout);
        }

        public State<TState, TData> GoTo(TState nextStateName)
        {
            return new State<TState, TData>(nextStateName, _currentState.StateData);
        }

        public State<TState, TData> Stay()
        {
            return GoTo(_currentState.StateName);
        }

        public State<TState, TData> Stop()
        {
            return Stop(Normal.Instance);
        }

        public State<TState, TData> Stop(Reason reason)
        {
            return Stop(reason, _currentState.StateData);
        }

        public State<TState, TData> Stop(Reason reason, TData stateData)
        {
            return Stay().Using(stateData).WithStopReason(reason);
        }

        public sealed class TransformHelper
        {
            public TransformHelper(StateFunction func)
            {
                Func = func;
            }

            public StateFunction Func { get; }

            public StateFunction Using(Func<State<TState, TData>, State<TState, TData>> andThen)
            {
                StateFunction continuedDelegate = @event => andThen.Invoke(Func.Invoke(@event));
                return continuedDelegate;
            }
        }

        public TransformHelper Transform(StateFunction func) => new TransformHelper(func);


        public void SetTimer(string name, object msg, TimeSpan timeout, bool repeat = false)
        {
            fsm_context.SetTimer(name, msg, timeout, repeat);
        }

        /// <summary>
        /// Cancel a named <see cref="StraletActor.Timer"/>, ensuring that the message is not subsequently delivered (no race.)
        /// </summary>
        /// <param name="name">The name of the timer to cancel.</param>
        public void KillTimer(string name)
        {
            fsm_context.KillTimer(name);
        }

        /// <summary>
        /// Determines whether the named timer is still active. Returns true 
        /// unless the timer does not exist, has previously been cancelled, or
        /// if it was a single-shot timer whose message was already received.
        /// </summary>
        /// <param name="name">TBD</param>
        /// <returns>TBD</returns>
        public bool IsTimerActive(string name)
        {
            return fsm_context.IsTimerActive(name);
        }

        /// <summary>
        /// Set the state timeout explicitly. This method can be safely used from
        /// within a state handler.
        /// </summary>
        /// <param name="state">TBD</param>
        /// <param name="timeout">TBD</param>
        public void SetStateTimeout(TState state, TimeSpan? timeout)
        {
            _stateTimeouts[state] = timeout;
        }


        /// <summary>
        /// Set handler which is called upon each state transition
        /// </summary>
        /// <param name="transitionHandler">TBD</param>
        public void OnTransition(TransitionHandler transitionHandler)
        {
            _transitionEvent.Add(transitionHandler);
        }

        /// <summary>
        /// Set the handler which is called upon termination of this FSM actor. Calling this
        /// method again will overwrite the previous contents.
        /// </summary>
        /// <param name="terminationHandler">TBD</param>
        public void OnTermination(Action<StopEvent<TState, TData>> terminationHandler)
        {
            _terminateEvent = terminationHandler;
        }

        /// <summary>
        /// Set handler which is called upon reception of unhandled FSM messages. Calling
        /// this method again will overwrite the previous contents.
        /// </summary>
        /// <param name="stateFunction">TBD</param>
        public void WhenUnhandled(StateFunction stateFunction)
        {
            HandleEvent = OrElse(stateFunction, HandleEventDefault);
        }

        // Post ?
        public void PostEvent(Object any)
        {
            fsm_context.PostEvent(any);
        }

        public void Initialize()
        {
            if (_currentState != null)
                MakeTransition(_currentState);
            else
                throw new IllegalStateException("You must call StartWith before calling Initialize.");
        }

        public TState StateName
        {
            get
            {
                if (_currentState != null)
                    return _currentState.StateName;
                throw new IllegalStateException("You must call StartWith before calling StateName.");
            }
        }

        public TData StateData
        {
            get
            {
                if (_currentState != null)
                    return _currentState.StateData;
                throw new IllegalStateException("You must call StartWith before calling StateData.");
            }
        }

        /// <summary>
        /// Return next state data (available in <see cref="OnTransition"/> handlers)
        /// </summary>
        public TData NextStateData
        {
            get
            {
                if (_nextState != null)
                    return _nextState.StateData;
                throw new InvalidOperationException("NextStateData is only available during OnTransition");
            }
        }

        //#endregion

        #region Internal implementation details

        /// <summary>
        /// FSM state data and current timeout handling
        /// </summary>
        private State<TState, TData> _currentState;

        //private Timer _stateTimout;
        private State<TState, TData> _nextState;
        //private int timer_id = 0;

        /// <summary>
        /// State definitions
        /// </summary>
        private readonly Dictionary<TState, StateFunction> _stateFunctions = new Dictionary<TState, StateFunction>();
        private readonly Dictionary<TState, TimeSpan?> _stateTimeouts = new Dictionary<TState, TimeSpan?>();

        private void Register(TState name, StateFunction function, TimeSpan? timeout)
        {
            StateFunction stateFunction;
            if (_stateFunctions.TryGetValue(name, out stateFunction))
            {
                _stateFunctions[name] = OrElse(stateFunction, function);
                _stateTimeouts[name] = _stateTimeouts[name] ?? timeout;
            }
            else
            {
                _stateFunctions.Add(name, function);
                _stateTimeouts.Add(name, timeout);
            }
        }

        /// <summary>
        /// Unhandled event handler
        /// </summary>
        private StateFunction HandleEventDefault
        {
            get
            {
                return delegate (Event<TData> @event)
                {
                    //_log.Warning("unhandled event {0} in state {1}", @event.FsmEvent, StateName);
                    return Stay();
                };
            }
        }

        private StateFunction _handleEvent;

        private StateFunction HandleEvent
        {
            get { return _handleEvent ?? (_handleEvent = HandleEventDefault); }
            set { _handleEvent = value; }
        }


        /// <summary>
        /// Termination handling
        /// </summary>
        private Action<StopEvent<TState, TData>> _terminateEvent = @event => { };

        /// <summary>
        /// Transition handling
        /// </summary>
        private readonly IList<TransitionHandler> _transitionEvent = new List<TransitionHandler>();

        private void HandleTransition(TState previous, TState next)
        {
            foreach (var tran in _transitionEvent)
            {
                tran.Invoke(previous, next);
            }
        }

        /// <summary>
        /// C# port of Scala's orElse method for partial function chaining.
        /// 
        /// See http://scalachina.com/api/scala/PartialFunction.html
        /// </summary>
        /// <param name="original">The original <see cref="StateFunction"/> to be called</param>
        /// <param name="fallback">The <see cref="StateFunction"/> to be called if <paramref name="original"/> returns null</param>
        /// <returns>A <see cref="StateFunction"/> which combines both the results of <paramref name="original"/> and <paramref name="fallback"/></returns>
        private static StateFunction OrElse(StateFunction original, StateFunction fallback)
        {
            StateFunction chained = delegate (Event<TData> @event)
            {
                var originalResult = original.Invoke(@event);
                if (originalResult == null) return fallback.Invoke(@event);
                return originalResult;
            };

            return chained;
        }

        #endregion

        public bool Receive(object message)
        {
            if (message != null)
                ProcessMsg(message);
            return true;
        }

        private void ProcessMsg(object any)
        {
            var fsmEvent = new Event<TData>(any, _currentState.StateData);
            ProcessEvent(fsmEvent);
        }

        private void ProcessEvent(Event<TData> fsmEvent)
        {
            var stateFunc = _stateFunctions[_currentState.StateName];
            var oldState = _currentState;

            State<TState, TData> nextState = null;

            if (stateFunc != null)
            {
                nextState = stateFunc(fsmEvent);
            }

            if (nextState == null)
            {
                nextState = HandleEvent(fsmEvent);
            }

            ApplyState(nextState);
        }

        private void ApplyState(State<TState, TData> nextState)
        {
            if (nextState.StopReason == null)
            {
                MakeTransition(nextState);
            }
            else
            {
                Terminate(nextState);
                fsm_context.Stop(nextState.StopReason);
            }
        }

        private void MakeTransition(State<TState, TData> nextState)
        {
            if (!_stateFunctions.ContainsKey(nextState.StateName))
            {
                Terminate(Stay().WithStopReason(new Failure($"Next state {nextState.StateName} does not exist")));
            }
            else
            {
                if (!_currentState.StateName.Equals(nextState.StateName))// || nextState.Notifies)
                {
                    _nextState = nextState;
                    HandleTransition(_currentState.StateName, nextState.StateName);
                    _nextState = null;
                }
                _currentState = nextState;

                var timeout = _currentState.Timeout ?? _stateTimeouts[_currentState.StateName];
                if (timeout.HasValue)
                {
                    var t = timeout.Value;
                    if (t < TimeSpan.MaxValue)
                    {
                        //_timeoutFuture = Context.System.Scheduler.ScheduleTellOnceCancelable(t, Context.Self, new TimeoutMarker(_generation), Context.Self);
                    }
                }
            }
        }

        private void Terminate(State<TState, TData> upcomingState)
        {
            if (_currentState.StopReason == null)
            {
                var reason = upcomingState.StopReason;
                _currentState = upcomingState;
                fsm_context.ClearTimers();

                //if (this._stateTimout != null)
                //{
                //    this.fsm_context.KillTimer(this._stateTimout.Id);
                //    this._stateTimout = null;
                //}

                var stopEvent = new StopEvent<TState, TData>(reason, _currentState.StateName, _currentState.StateData);
                _terminateEvent(stopEvent);
            }
        }


        ///// <summary>
        ///// By default, <see cref="Failure"/> is logged at error level and other
        ///// reason types are not logged. It is possible to override this behavior.
        ///// </summary>
        ///// <param name="reason">TBD</param>
        //protected virtual void LogTermination(Reason reason)
        //{
        //}
    }
}
