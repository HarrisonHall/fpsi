use crate::event::Event;

pub struct CommController {}

impl CommController {
    pub fn new() -> Self {
        CommController {}
    }
    /// Filter Send and Recv messages
    /// Message is expectected to be a Send or Recv
    pub fn filter(&mut self, message: &Event) -> Event {
        // TODO
        return Event::None;
    }
}
