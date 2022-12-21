use std::default::Default;

pub struct Config {
    pub debug: bool,                    // Debug flag
    pub session_name: String,           // Session name
    pub node_name: String,              // Communication node name
    pub agg_per_second: f64,            // Number of aggregations to target per second
    pub global_channel_size: usize,     // Size of global event channel
    pub plugin_channel_size: usize,     // Size of event channel per plugin
    pub event_loop_channel_size: usize, // Size of event queue to parse per Session::run() loop
}

impl Default for Config {
    fn default() -> Self {
        Config {
            debug: false,
            session_name: String::from("FPSI"),
            node_name: String::from("FPSI_NODE_DEFAULT"),
            agg_per_second: 4.0,
            global_channel_size: 1024,
            plugin_channel_size: 256,
            event_loop_channel_size: 32,
        }
    }
}
