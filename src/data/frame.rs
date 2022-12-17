use std::clone::Clone;
use std::time::SystemTime;

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

impl Clone for Frame {
    fn clone(&self) -> Self {
        Frame {
            id: self.id,
            source: self.source.clone(),
            time: self.time.clone(),
            data: self.data.clone(),
        }
    }
}
