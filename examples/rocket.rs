use fpsi::prelude::*;

enum RocketSources {
    Telemetry,
    SensorBoard,
}

#[derive(Serialize, Deserialize, Clone)]
enum RocketState {
    Idle,
    Up,
    Apogee,
    Down,
    Recovery,
}

impl fpsi::State<'_> for RocketState {
    fn tick() -> fpsi::Tick {
        0
    }
}

#[derive(Serialize, Deserialize, Clone)]
struct Altitude {
    height: f32,
}

impl Altitude {
    fn new(height: f32) -> Self {
        Self { height }
    }
}

impl fpsi::Frame<'_, RocketSources> for Altitude {
    fn tick() -> fpsi::Tick {
        0
    }

    fn source() -> RocketSources {
        RocketSources::SensorBoard
    }
}

fn main() {
    println!("Test!");
}
