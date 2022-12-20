use crate::config::Config;
use crate::event::Event;

pub struct CommController {}

impl CommController {
    pub fn new(_config: &Config) -> Self {
        CommController {}
    }
    /// Filter Send and Recv messages
    /// Message is expectected to be a Send or Recv
    pub fn filter(&mut self, _message: &Event) -> Event {
        // TODO
        return Event::None;
    }
}
