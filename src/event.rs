use crate::data;

#[derive(Clone)]
pub enum Event {
    None,                               // No event, ignore
    State { state: String, value: u8 }, // TODO
    RawData(data::Frame),               // Raw data frame
    AggData(data::Frame),               // Aggregate data frame
    Send {},                            // TODO
    Recv {},                            // TODO
    Die,                                // Kill FPSI
}
