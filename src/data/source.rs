use std::collections::vec_deque::VecDeque;
use std::vec::Vec;

use crate::data::Frame;
use crate::util::Shareable;

/// Holds datafarmes in raw and aggregate form
pub struct Source {
    pub name: String,

    last_raw: Option<Shareable<Frame>>,
    last_agg: Option<Shareable<Frame>>,

    raw_frames: VecDeque<Shareable<Frame>>,
    agg_frames: VecDeque<Shareable<Frame>>,
    max_queue_len: usize,
}

impl Source {
    pub fn new<'a>(name: &'a str) -> Self {
        Source {
            name: String::from(name),
            last_raw: None,
            last_agg: None,
            raw_frames: VecDeque::new(),
            agg_frames: VecDeque::new(),
            max_queue_len: 10,
        }
    }

    pub fn track_raw(&mut self, frame: Frame) -> () {
        self.raw_frames.push_back(Shareable::new(frame));
        if self.raw_frames.len() > self.max_queue_len {
            self.raw_frames.pop_front();
        }
    }

    pub fn track_agg(&mut self, frame: Frame) -> () {
        self.agg_frames.push_back(Shareable::new(frame));
        if self.agg_frames.len() > self.max_queue_len {
            self.agg_frames.pop_front();
        }
    }

    pub fn get_new_raws(&mut self) -> Vec<Shareable<Frame>> {
        let mut new_raws = Vec::new();

        for frame_ref in self.raw_frames.iter() {
            match &self.last_raw {
                None => {
                    new_raws.push(frame_ref.clone());
                }
                Some(_last_raw) => {
                    self.last_raw = None;
                    // Now we'll push everything after this
                }
            }
        }

        if new_raws.len() > 0 {
            self.last_raw = Some(new_raws[new_raws.len() - 1].clone());
        }
        return new_raws;
    }

    pub fn get_new_aggs(&mut self) -> Vec<Shareable<Frame>> {
        let mut new_aggs = Vec::new();

        for frame_ref in self.agg_frames.iter() {
            match &self.last_agg {
                None => {
                    new_aggs.push(frame_ref.clone());
                }
                Some(_last_agg) => {
                    self.last_agg = None;
                    // Now we'll push everything after this
                }
            }
        }

        if new_aggs.len() > 0 {
            self.last_agg = Some(new_aggs[new_aggs.len() - 1].clone());
        }
        return new_aggs;
    }

    pub fn close(&mut self) -> () {
        self.last_raw = None;
        self.last_agg = None;
        self.raw_frames.clear();
        self.agg_frames.clear();
        println!("Closed {}", self.name);
    }
}
