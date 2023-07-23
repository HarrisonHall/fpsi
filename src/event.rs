///! Event
use crate::data;
use std::marker::PhantomData;

#[derive(Clone)]
pub enum Event<'e, Source, F: data::Frame<'e, Source>, S: data::State<'e>> {
    _Phantom(PhantomData<(&'e F, &'e S, &'e Source)>),
    State(S),
    Raw(F),
    Agg(F),
    Shutdown,
}

pub trait Handler {
    fn init(&mut self); // TODO queue
    fn produce(&self);
    fn aggregate(&self);
    fn consume(&self);
}
