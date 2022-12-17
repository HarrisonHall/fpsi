use std::collections::HashMap;
use std::vec::Vec;

use crate::data;
use crate::session::Session;
use crate::util::Shareable;

pub enum Signal {
    PreAggregate {
        data: HashMap<String, Vec<Shareable<data::Frame>>>,
    },
    PostAggregate {
        data: HashMap<String, Vec<Shareable<data::Frame>>>,
    },
    StateChange {
        key: String,
        value: usize,
    },
    // TODO Broadcast
    // TODO Send
    // TODO Receive
}

pub trait Plugin {
    fn load(&mut self) -> Result<(), String> {
        Ok(())
    }
    fn process_signal(&mut self, session: &mut Session, signal: &Signal) -> Result<(), String> {
        Ok(())
    }
    fn unload(&mut self) -> Result<(), String> {
        Ok(())
    }
}
#[cfg(test)]
mod tests {
    use super::*;
}
