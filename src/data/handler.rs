use std::ops::Drop;
use std::vec::Vec;

use crate::data::{Frame, Source};
use crate::util::Shareable;

const DEFAULT_AGG_PER_SEC: f64 = 4.0;

/// Holds multiple sources
pub struct Handler {
    sources: Vec<Shareable<Source>>,
    pub agg_per_second: f64,
    closed: bool,
}

impl Handler {
    pub fn new() -> Self {
        Handler {
            sources: Vec::new(),
            agg_per_second: DEFAULT_AGG_PER_SEC,
            closed: false,
        }
    }

    pub fn create_source<'a>(&mut self, name: &'a str) -> bool {
        // Check if closed
        if self.closed {
            return false;
        }
        // Check if already exists
        for source in self.sources.iter() {
            if let Ok(source) = source.read() {
                if source.name == name {
                    return true;
                }
            }
        }
        // Create data source
        self.sources.push(Shareable::new(Source::new(name)));
        return true;
    }

    pub fn close_sources(&mut self) -> () {
        for source in self.sources.iter_mut() {
            if let Ok(mut source) = source.write() {
                source.close();
            }
        }
    }

    pub fn get_source<'a>(&self, name: &'a str) -> Option<Shareable<Source>> {
        for source_ref in self.sources.iter() {
            if let Ok(source) = source_ref.read() {
                if source.name == name {
                    return Some(source_ref.clone());
                }
            }
        }
        return None;
    }

    pub fn get_source_names(&self) -> Vec<String> {
        let mut names = Vec::new();
        for source in self.sources.iter() {
            if let Ok(source) = source.read() {
                names.push(source.name.clone());
            }
        }
        return names;
    }
}

impl Drop for Handler {
    fn drop(&mut self) {
        self.close_sources();
    }
}
