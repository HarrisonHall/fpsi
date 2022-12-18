use std::clone::Clone;
use std::fmt;
use std::time::SystemTime;

use serde_json;

use log::*;

/// Single instance of data, can be raw or agg
pub struct Frame {
    pub id: u64,
    pub source: String,
    pub time: SystemTime,
    pub data: serde_json::Value,
}

impl Frame {
    pub fn new(source: &str, data: Option<serde_json::Value>) -> Self {
        trace!("New frame created: {}", source);
        Frame {
            id: 0,
            source: String::from(source),
            time: SystemTime::now(),
            data: match data {
                Some(data) => data,
                None => serde_json::Value::Null,
            },
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

impl fmt::Display for Frame {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        write!(f, "<Frame {}>", self.data)
    }
}
