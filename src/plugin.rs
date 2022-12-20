use std::fmt::Display;
use std::sync::Arc;
use std::vec::Vec;

use crossbeam_channel::{Receiver, Sender};

use crate::data;
use crate::event::Event;

pub trait Plugin: Send + Sync + Display {
    fn load(&mut self) -> Result<(), String> {
        Ok(())
    }
    fn unload(&mut self) -> Result<(), String> {
        Ok(())
    }
    fn process_events(
        &self,
        _event_consumer: Receiver<Event>,
        _event_producer: Sender<Event>,
    ) -> Result<(), String> {
        Ok(())
    }
    fn pre_agg(
        &self,
        _raw_frames: Arc<Vec<data::Frame>>,
        _event_producer: Sender<Event>,
    ) -> Result<(), String> {
        Ok(())
    }
    fn post_agg(
        &self,
        _agg_frames: Arc<Vec<data::Frame>>,
        _event_producer: Sender<Event>,
    ) -> Result<(), String> {
        Ok(())
    }
}
