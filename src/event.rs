use serde_json;

use crate::data;
use crate::session::communication;

#[derive(Clone)]
pub enum Event {
    /// No event, ignore.
    None,
    /// State change.
    State {
        state: String,
        value: serde_json::Value,
    },
    /// Raw data frame, pre-aggregation.
    RawData(data::Frame),
    /// Aggregate data frame, post-aggregation.
    AggData(data::Frame),
    /// Communication message between other fpsi nodes.
    Communication(communication::CommMessage),
    /// Kill message.
    /// Send to kill FPSI, receive indicates that a thread should die.
    Die,
}
