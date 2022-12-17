use std::vec::Vec;

use crossbeam_channel::{bounded, Receiver, Sender};

use crate::event::Event;

const MESSAGE_QUEUE_SIZE: usize = 256;
const DEFAULT_AGG_PER_SEC: f64 = 4.0;

/// Holds multiple sources
pub struct Handler {
    event_producer: Sender<Event>,
    event_consumer: Receiver<Event>,
    pub agg_per_second: f64,
}

impl Handler {
    pub fn new() -> Self {
        let (producer, consumer) = bounded(MESSAGE_QUEUE_SIZE);
        Handler {
            event_producer: producer,
            event_consumer: consumer,
            agg_per_second: 4.0,
        }
    }

    /// Get events from our events consumer up to MESSAGE_QUEUE_SIZE
    pub fn get_events(&self) -> Vec<Event> {
        let mut current_events = Vec::new();
        loop {
            if current_events.len() >= MESSAGE_QUEUE_SIZE {
                break;
            }
            match self.event_consumer.try_recv() {
                Ok(event) => current_events.push(event),
                Err(_) => break,
            }
        }
        return current_events;
    }

    pub fn get_event_producer(&self) -> Sender<Event> {
        self.event_producer.clone()
    }
}
