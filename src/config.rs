use config;
pub use config::Config;
use serde;

pub const DEBUG: bool = false;
pub const DEFAULT_SESSION_NAME: &'static str = "FPSI";
pub const DEFAULT_AGG_PER_SEC: f64 = 4.0;

pub fn config_from_file(filename: &str) -> Config {
    let mut builder = config::Config::builder();
    builder = builder.add_source(config::File::new(filename, config::FileFormat::Yaml));
    return match builder.build() {
        Ok(config) => config,
        Err(_) => Config::default(),
    };
}

pub trait ConfigExtension {
    fn get_default<'de, T: serde::de::Deserialize<'de>>(&self, key: &str, default: T) -> T;
}

impl ConfigExtension for Config {
    fn get_default<'de, T: serde::de::Deserialize<'de>>(&self, key: &str, default: T) -> T {
        match self.get(key) {
            Ok(val) => val,
            Err(_) => default,
        }
    }
}
