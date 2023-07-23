use fpsi::prelude::*;
use std::sync::Arc;

#[derive(Serialize, Deserialize, Clone, Debug)]
enum RocketSource {
    Calculation,
    Telemetry,
    SensorBoard,
    Simulation,
}

impl Source for RocketSource {}

#[derive(Serialize, Deserialize, Clone, Debug)]
enum FlightState {
    Idle,
    Up,
    Apogee,
    Down,
    Recovery,
}

#[derive(Serialize, Deserialize, Clone, Debug)]
enum RocketState {
    Locked(bool),
    FlightState(FlightState),
}

impl fpsi::State for RocketState {}

#[derive(Serialize, Deserialize, Clone, Debug)]
enum RocketDataType {
    Altitude(f32),
    Gyro { pitch: f32, yaw: f32, roll: f32 },
}

#[derive(Serialize, Deserialize, Clone, Debug)]
struct RocketData {
    source: RocketSource,
    data: RocketDataType,
}

impl fpsi::Frame<RocketSource> for RocketData {
    fn source(&self) -> RocketSource {
        self.source.clone()
    }
}

type RocketEvent = fpsi::Event<RocketSource, RocketData, RocketState>;

struct AltitudeHandler {}

impl AltitudeHandler {
    fn new() -> Self {
        Self {}
    }
}

impl fpsi::Handler<RocketSource, RocketData, RocketState> for AltitudeHandler {
    /// Produce raw data frames
    fn produce(&self, ctx: fpsi::HandlerContext<RocketSource, RocketData, RocketState>) {
        let mut altitude: f32 = 0.0;
        let mut max_iter: usize = 15;
        'producer: loop {
            match ctx.recv() {
                fpsi::Event::Shutdown => {
                    break 'producer;
                }
                _ => {}
            }

            max_iter -= 1;
            if max_iter == 0 {
                ctx.send(RocketEvent::Shutdown);
                break 'producer;
            }
            ctx.send(RocketEvent::Raw(RocketData {
                source: RocketSource::SensorBoard,
                data: RocketDataType::Altitude(altitude),
            }));
            altitude += 2.5;
            std::thread::sleep(std::time::Duration::from_secs_f32(0.4));
        }
    }
    /// Aggregrate raw data frames
    fn aggregate(&self, ctx: fpsi::HandlerContext<RocketSource, RocketData, RocketState>) {
        'aggregator: loop {
            match ctx.recv() {
                fpsi::Event::Shutdown => {
                    break 'aggregator;
                }
                fpsi::Event::Raws(raws) => {
                    let mut tot_alts: usize = 0;
                    let mut tot_altitude: f32 = 0.0;
                    for raw in raws.iter() {
                        match &raw.data {
                            RocketDataType::Altitude(alt) => {
                                tot_alts += 1;
                                tot_altitude += alt;
                            }
                            _ => {}
                        }
                    }
                    if tot_alts > 0 {
                        ctx.send(RocketEvent::Agg(RocketData {
                            source: RocketSource::Calculation,
                            data: RocketDataType::Altitude(tot_altitude / tot_alts as f32),
                        }));
                    }
                }
                _ => {}
            }
        }
    }
    /// Consume aggregate data frames and state changes
    fn consume(&self, ctx: fpsi::HandlerContext<RocketSource, RocketData, RocketState>) {
        'consumer: loop {
            match ctx.recv() {
                fpsi::Event::Shutdown => {
                    break 'consumer;
                }
                fpsi::Event::Agg(agg) => {
                    println!("Aggregate: {:?}", agg);
                }
                _ => {}
            }
        }
    }
}

fn main() {
    println!("RocketTest!");
    let mut node = Node::new();
    node.register_event_handler(Arc::new(AltitudeHandler::new()));
    node.run().ok();
}
