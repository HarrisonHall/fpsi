use serde_json;

use crate::data;

#[derive(Clone)]
pub enum Event {
    None, // No event, ignore
    State {
        state: String,
        value: serde_json::Value,
    }, // TODO
    RawData(data::Frame), // Raw data frame
    AggData(data::Frame), // Aggregate data frame
    Send {}, // TODO
    Recv {}, // TODO
    Die,  // Kill FPSI
}
