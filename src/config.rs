use std::default::Default;

pub const DEBUG: bool = false;
pub const DEFAULT_SESSION_NAME: &'static str = "FPSI";
pub const DEFAULT_AGG_PER_SEC: f64 = 4.0;

pub struct Config {
    pub debug: bool,
    pub session_name: String,
    pub agg_per_second: f64,
    pub channel_size: usize,
}

impl Default for Config {
    fn default() -> Self {
        Config {
            debug: true,
            session_name: String::from(""),
            agg_per_second: 0.25,
            channel_size: u8::MAX as usize,
        }
    }
}
