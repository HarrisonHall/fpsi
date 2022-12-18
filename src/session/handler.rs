use std::time::SystemTime;
use std::vec::Vec;

use crossbeam_channel::{bounded, Receiver, Sender};

use crate::data;
use crate::event::Event;

const MESSAGE_QUEUE_SIZE: usize = 256;
const DEFAULT_AGG_PER_SEC: f64 = 4.0;

/// Holds multiple sources
pub struct Handler {
    event_producer: Sender<Event>,
    event_consumer: Receiver<Event>,
    pub agg_per_second: f64,
    pub last_agg_time: SystemTime,
    latest_raw_frames: Vec<data::Frame>,
    latest_agg_frames: Vec<data::Frame>,
    pub next_step: AggregationStep,
}

impl Handler {
    pub fn new() -> Self {
        let (producer, consumer) = bounded(MESSAGE_QUEUE_SIZE);
        Handler {
            event_producer: producer,
            event_consumer: consumer,
            agg_per_second: 4.0,
            last_agg_time: SystemTime::now(),
            latest_raw_frames: Vec::new(),
            latest_agg_frames: Vec::new(),
            next_step: AggregationStep::PreAgg,
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

    pub fn time_delta_has_passed(&self) -> bool {
        match self.last_agg_time.elapsed() {
            //Ok(duration) => duration.as_secs_f64() > 1.0 / self.handler.agg_per_second,
            Ok(duration) => duration.as_secs_f64() > 1.0 / self.agg_per_second,
            Err(_) => false,
        }
    }

    pub fn started_pre_aggs(&mut self) {
        self.next_step = AggregationStep::PreAgg;
        self.last_agg_time = SystemTime::now();
    }

    pub fn started_post_aggs(&mut self) {
        self.next_step = AggregationStep::PostAgg;
    }

    pub fn add_raw_frame(&mut self, frame: data::Frame) {
        self.latest_raw_frames.push(frame);
    }

    pub fn add_agg_frame(&mut self, frame: data::Frame) {
        self.latest_agg_frames.push(frame);
    }

    pub fn dump_raw_frames(&mut self) -> Vec<data::Frame> {
        let raw_frames = self.latest_raw_frames.clone();
        self.latest_raw_frames.clear();
        return raw_frames;
    }

    pub fn dump_agg_frames(&mut self) -> Vec<data::Frame> {
        // Normalize timestamps
        let current_time = SystemTime::now();
        for frame in self.latest_agg_frames.iter_mut() {
            frame.time = current_time;
        }
        let agg_frames = self.latest_agg_frames.clone();
        self.latest_agg_frames.clear();
        return agg_frames;
    }
}

#[derive(PartialEq)]
pub enum AggregationStep {
    PreAgg,
    PostAgg,
}
