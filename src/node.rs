use std::sync::Arc;
use std::thread;
use std::vec::Vec;

use crate::event;

// pub use crossbeam_channel::{bounded, Receiver, Sender};
use log::*;

pub struct Node {
    name: String,
    event_handlers: Vec<Box<dyn event::Handler>>,
    // ticks_per_sec: f32,
    // config: Config,
    // handler: handler::Handler,
    // communicator: CommController,
    // plugins: Vec<PluginThreadGroup>,
}

impl Node {
    pub fn new(name: &'_ str) -> Self {
        Self {
            name: name.to_string(),
            event_handlers: Vec::new(),
            // ticks_per_sec: 1.0,
        }
    }

    pub fn register_event_handler(&mut self, handler: Box<dyn event::Handler>) {
        self.event_handlers.push(handler);
    }

    pub fn run(&mut self) -> Result<(), ()> {
        // Start event threads

        // Loop event aggregation and data aggregation steps

        // Join all threads

        Ok(())
    }
}
