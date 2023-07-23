///! Event
use crate::data;
pub use crossbeam_channel::{bounded, Receiver, Sender};
pub use std::marker::PhantomData;
use std::sync::Arc;
use std::sync::RwLock;

// #[derive(Send)]
pub enum Event<So, Fr, St>
where
    So: data::Source,
    Fr: data::Frame<So>,
    St: data::State,
{
    #[allow(dead_code)]
    __Phantom(PhantomData<So>),
    State(St),
    Raw(Fr),
    Agg(Fr),
    Shutdown,
}

unsafe impl<So, Fr, St> Send for Event<So, Fr, St>
where
    So: data::Source,
    Fr: data::Frame<So>,
    St: data::State,
{
}

pub trait Handler<So, Fr, St>: Send + Sync
where
    So: data::Source,
    Fr: data::Frame<So>,
    St: data::State,
{
    /// Produce raw data frames
    fn produce(&self, ctx: HandlerContext<So, Fr, St>);
    /// Aggregrate raw data frames
    fn aggregate(&self, ctx: HandlerContext<So, Fr, St>);
    /// Consume aggregate data frames and state changes
    fn consume(&self, ctx: HandlerContext<So, Fr, St>);
}

#[derive(Clone)]
pub struct HandlerContext<So, Fr, St>
where
    So: data::Source,
    Fr: data::Frame<So>,
    St: data::State,
{
    events: Sender<Event<So, Fr, St>>,
    states: Receiver<St>,
    raws: Option<Receiver<data::FrameVec<Fr>>>,
    aggs: Option<Receiver<Fr>>,
    shutdown: Arc<RwLock<bool>>,
}

impl<So, Fr, St> HandlerContext<So, Fr, St>
where
    So: data::Source,
    Fr: data::Frame<So>,
    St: data::State,
{
    pub fn new(
        events: Sender<Event<So, Fr, St>>,
        states: Receiver<St>,
        raws: Option<Receiver<data::FrameVec<Fr>>>,
        aggs: Option<Receiver<Fr>>,
        shutdown: Arc<RwLock<bool>>,
    ) -> Self {
        Self {
            events,
            states,
            raws,
            aggs,
            shutdown,
        }
    }

    pub fn send_event(&self, event: Event<So, Fr, St>) {
        self.events.send(event).ok();
    }

    pub fn state(&self) -> Option<St> {
        match self.states.recv() {
            Ok(state) => Some(state),
            Err(_) => None,
        }
    }

    pub fn raws(&self) -> Option<data::FrameVec<Fr>> {
        match &self.raws {
            Some(raw) => match raw.recv() {
                Ok(frames) => Some(frames),
                Err(_) => None,
            },
            None => None,
        }
    }

    pub fn aggregate(&self) -> Option<Fr> {
        match &self.aggs {
            Some(agg) => match agg.recv() {
                Ok(frame) => Some(frame),
                Err(_) => None,
            },
            None => None,
        }
    }

    pub fn shutdown(&self) -> bool {
        match self.shutdown.read() {
            Ok(guard) => *guard,
            Err(_) => true,
        }
    }
}

/*
unsafe impl<So, Fr, St> Send for HandlerContext<So, Fr, St>
where
    So: data::Source,
    Fr: data::Frame<So>,
    St: data::State,
{
}

unsafe impl<So, Fr, St> Sync for HandlerContext<So, Fr, St>
where
    So: data::Source,
    Fr: data::Frame<So>,
    St: data::State,
{
}
*/
