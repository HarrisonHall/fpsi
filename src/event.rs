///! Event
use crate::data;
pub use crossbeam_channel::{bounded, Receiver, Sender};
pub use std::marker::PhantomData;
use std::sync::Arc;

#[derive(Clone)]
pub enum Event<So, Fr, St>
where
    So: data::Source,
    Fr: data::Frame<So>,
    St: data::State,
{
    #[allow(dead_code)]
    __Phantom(PhantomData<So>),
    None,
    State(St),
    Raw(Fr),
    Raws(Arc<Vec<Fr>>),
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
    send: Sender<Event<So, Fr, St>>,
    recv: Receiver<Event<So, Fr, St>>,
}

impl<So, Fr, St> HandlerContext<So, Fr, St>
where
    So: data::Source,
    Fr: data::Frame<So>,
    St: data::State,
{
    pub fn new(send: Sender<Event<So, Fr, St>>, recv: Receiver<Event<So, Fr, St>>) -> Self {
        Self { send, recv }
    }

    pub fn send(&self, event: Event<So, Fr, St>) -> () {
        self.send.send(event).ok();
    }

    pub fn recv(&self) -> Event<So, Fr, St> {
        match self.recv.try_recv() {
            Ok(event) => event,
            Err(_) => Event::None,
        }
    }
}
