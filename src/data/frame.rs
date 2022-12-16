use std::time::SystemTime;

use crate::util::Shareable;

/// Single instance of data, can be raw or agg
pub struct Frame {
    pub id: u64,
    pub source: String,
    pub time: SystemTime,
    pub data: u64, // TODO
}

impl Frame {
    pub fn new() -> Self {
        Frame {
            id: 0,
            source: String::from(""),
            time: SystemTime::now(),
            data: 0,
        }
    }
}
