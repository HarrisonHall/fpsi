use fpsi::prelude::*;
use std::sync::Arc;

#[derive(Serialize, Deserialize, Clone)]
enum RocketSource {
    Telemetry,
    SensorBoard,
    Simulation,
}

impl Source for RocketSource {}

#[derive(Serialize, Deserialize, Clone)]
enum RocketState {
    Idle,
    Up,
    Apogee,
    Down,
    Recovery,
}

impl fpsi::State for RocketState {}

#[derive(Serialize, Deserialize, Clone)]
struct Altitude {
    height: f32,
}

impl Altitude {
    fn new(height: f32) -> Self {
        Self { height }
    }
}

#[derive(Serialize, Deserialize, Clone)]
enum RocketDataType {
    Altitude(Altitude),
}

#[derive(Serialize, Deserialize, Clone)]
struct RocketData {
    // tick: fpsi::Tick,
    source: RocketSource,
    data: RocketDataType,
}

impl fpsi::Frame<RocketSource> for RocketData {
    fn source(&self) -> RocketSource {
        self.source.clone()
    }
}

struct TelemetryHandler {
    // sender: Option<fpsi::Sender<Event<RocketSource, RocketData, RocketState>>>,
}

impl TelemetryHandler {
    fn new() -> Self {
        Self {}
    }
}

type RocketEvent = fpsi::Event<RocketSource, RocketData, RocketState>;

impl fpsi::Handler<RocketSource, RocketData, RocketState> for TelemetryHandler {
    /// Produce raw data frames
    fn produce(&self, ctx: fpsi::HandlerContext<RocketSource, RocketData, RocketState>) {
        let mut altitude: f32 = 0.0;
        let mut max_iter: usize = 5;
        while !ctx.shutdown() {
            max_iter -= 1;
            if max_iter == 0 {
                ctx.send_event(RocketEvent::Shutdown);
            }
            ctx.send_event(RocketEvent::Raw(RocketData {
                source: RocketSource::SensorBoard,
                data: RocketDataType::Altitude(Altitude::new(altitude)),
            }));
            // sender.send(RocketEvent::Raw(RocketData::Altitude(Altitude::new(
            // altitude,
            // ))));
            println!("Altitude: {}", altitude);
            altitude += 2.5;
            std::thread::sleep(std::time::Duration::from_secs_f32(0.6));
        }
    }
    /// Aggregrate raw data frames
    fn aggregate(&self, ctx: fpsi::HandlerContext<RocketSource, RocketData, RocketState>) {}
    /// Consume aggregate data frames and state changes
    fn consume(&self, ctx: fpsi::HandlerContext<RocketSource, RocketData, RocketState>) {}
}

fn main() {
    println!("RocketTest!");
    let mut node = Node::new();
    node.register_event_handler(Arc::new(TelemetryHandler::new()));
    node.run();
    // let t_handler = TelemetryHandler::init();
}
