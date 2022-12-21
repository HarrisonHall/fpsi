use std::sync::Arc;
use std::thread;
use std::vec::Vec;

pub use crossbeam_channel::{bounded, Receiver, Sender};
use log::*;

use crate::config::Config;
use crate::event::Event;
use crate::plugin::Plugin;
use crate::session::communication::CommController;
use crate::session::handler;

pub struct Session {
    config: Config,
    handler: handler::Handler,
    communicator: CommController,
    plugins: Vec<PluginThreadGroup>,
}

impl Session {
    pub fn new(config: Option<Config>) -> Self {
        let config = match config {
            Some(config) => config,
            None => Config::default(),
        };
        let handler = handler::Handler::new(&config);
        let communicator = CommController::new(&config);
        Session {
            config: config,
            handler: handler,
            communicator: communicator,
            plugins: Vec::new(),
        }
    }

    pub fn register_plugin<T: Plugin + 'static>(&mut self, plugin: T) {
        trace!("Registering plugin {}", plugin);
        let (producer, consumer) = bounded::<Event>(self.config.plugin_channel_size);
        self.plugins.push(PluginThreadGroup {
            plugin: Arc::new(plugin),
            event_thread: None,
            event_producer: producer,
            event_consumer: consumer,
            pre_agg_thread: None,
            post_agg_thread: None,
        });
    }

    pub fn run(&mut self) -> Result<(), ()> {
        // Start event threads
        trace!("Creating plugin event consumer threads");
        for plugin_ctx in self.plugins.iter_mut() {
            let plugin = plugin_ctx.plugin.clone();
            let consumer = plugin_ctx.event_consumer.clone();
            let event_producer = self.handler.get_event_producer();
            plugin_ctx.event_thread = Some(thread::spawn(move || {
                plugin.process_events(consumer, event_producer)
            }));
        }

        // Loop event aggregation and data aggregation steps
        trace!("Starting aggregation loop");
        'aggregation: loop {
            // Get latest events
            for event in self.handler.get_events().iter_mut() {
                match event {
                    Event::RawData(frame) => {
                        self.handler.add_raw_frame(frame.clone());
                    }
                    Event::AggData(frame) => {
                        self.handler.add_agg_frame(frame.clone());
                    }
                    Event::Communication(comm_message) => {
                        let comm_event = self.communicator.filter(&comm_message);
                        self.push_event_to_plugins(&comm_event);
                    }
                    Event::Die => {
                        trace!("Received Event::Die, exiting...");
                        self.push_event_to_plugins(event);
                        break 'aggregation;
                    }
                    _ => {
                        self.push_event_to_plugins(event);
                    }
                };
            }

            match self.handler.next_step {
                handler::AggregationStep::PreAgg => {
                    if self.handler.time_delta_has_passed() && !self.waiting_for_postaggs() {
                        self.start_pre_aggs();
                        self.handler.started_pre_aggs();
                    }
                }
                handler::AggregationStep::PostAgg => {
                    if !self.waiting_for_preaggs() {
                        self.start_post_aggs();
                        self.handler.started_post_aggs();
                    }
                }
            }
        }

        // Join all threads
        trace!("Joining all theads");
        self.join_plugin_threads();

        Ok(())
    }

    fn start_pre_aggs(&mut self) {
        trace!("Starting pre-aggregation step");
        // Get agg data from vec
        let pre_agg_frames = Arc::new(self.handler.dump_raw_frames());

        // Start postags
        for plugin_ctx in self.plugins.iter_mut() {
            let pre_agg_frames = pre_agg_frames.clone();
            let plugin = plugin_ctx.plugin.clone();
            let event_producer = self.handler.get_event_producer();
            plugin_ctx.pre_agg_thread = Some(thread::spawn(move || {
                plugin.pre_agg(pre_agg_frames, event_producer)
            }));
        }
    }

    fn start_post_aggs(&mut self) {
        trace!("Starting post-aggregation step");
        // Copy raw data from queue to vec
        let post_agg_frames = Arc::new(self.handler.dump_agg_frames());

        // Start preaggs
        for plugin_ctx in self.plugins.iter_mut() {
            let post_agg_frames = post_agg_frames.clone();
            let plugin = plugin_ctx.plugin.clone();
            let event_producer = self.handler.get_event_producer();
            plugin_ctx.post_agg_thread = Some(thread::spawn(move || {
                plugin.post_agg(post_agg_frames, event_producer)
            }));
        }
    }

    fn waiting_for_preaggs(&self) -> bool {
        !self
            .plugins
            .iter()
            .all(|plug| plug.thread_is_done(&plug.pre_agg_thread))
    }

    fn waiting_for_postaggs(&self) -> bool {
        !self
            .plugins
            .iter()
            .all(|plug| plug.thread_is_done(&plug.post_agg_thread))
    }

    /// Push an event to all plugin consumer threads
    fn push_event_to_plugins(&self, event: &Event) -> () {
        for plugin in self.plugins.iter() {
            if let Some(event_thread) = &plugin.event_thread {
                if !event_thread.is_finished() {
                    plugin.event_producer.send(event.clone()).ok();
                }
            }
        }
    }

    /// Join all plugin threads
    fn join_plugin_threads(&mut self) -> () {
        for plugin_ctx in self.plugins.iter_mut() {
            for thread in [
                plugin_ctx.event_thread.take(),
                plugin_ctx.pre_agg_thread.take(),
                plugin_ctx.post_agg_thread.take(),
            ] {
                if let Some(thread) = thread {
                    match thread.join() {
                        Ok(result) => match result {
                            Ok(()) => {}
                            Err(err) => {
                                error!("`{}` thread did not return OK: {}", plugin_ctx.plugin, err);
                            }
                        },
                        Err(_err) => {
                            error!(
                                "`{}` failed to complete thread properly due to panic",
                                plugin_ctx.plugin
                            );
                        }
                    }
                }
            }
        }
    }
}

struct PluginThreadGroup {
    plugin: Arc<dyn Plugin>,
    event_thread: Option<thread::JoinHandle<Result<(), String>>>,
    event_producer: Sender<Event>,
    event_consumer: Receiver<Event>,
    pre_agg_thread: Option<thread::JoinHandle<Result<(), String>>>,
    post_agg_thread: Option<thread::JoinHandle<Result<(), String>>>,
}

impl PluginThreadGroup {
    fn thread_is_done<T>(&self, thread: &Option<thread::JoinHandle<T>>) -> bool {
        match thread {
            Some(thread) => return thread.is_finished(),
            None => return true,
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use std::fmt;
    use std::time::Duration;

    use crate::data;

    struct PseudoPlug {
        x: u8,
    }
    impl Plugin for PseudoPlug {
        fn load(&mut self) -> Result<(), String> {
            Ok(())
        }
        fn process_events(
            &self,
            event_consumer: Receiver<Event>,
            event_producer: Sender<Event>,
        ) -> Result<(), String> {
            if self.x == 0 {
                event_producer
                    .send(Event::State {
                        state: "PseudoState".to_string(),
                        value: serde_json::Value::Number(serde_json::Number::from(self.x)),
                    })
                    .ok();
                thread::sleep(Duration::new(1, 0));
                event_producer.send(Event::Die).ok();
            }
            loop {
                // Send out new data for current step
                event_producer
                    .send(Event::RawData(data::Frame::new("pseudo_src", None)))
                    .ok();
                match event_consumer.recv() {
                    Ok(event) => match event {
                        Event::Die => {
                            break;
                        }
                        _ => {}
                    },
                    Err(_) => break,
                }
            }
            Ok(())
        }
        fn pre_agg(
            &self,
            raw_frames: Arc<Vec<data::Frame>>,
            event_producer: Sender<Event>,
        ) -> Result<(), String> {
            for frame in raw_frames.iter() {
                println!("{} Aggregating {}", self.x, frame);
                if self.x == 0 {
                    event_producer.send(Event::AggData(frame.clone())).ok();
                }
            }
            Ok(())
        }
        fn post_agg(
            &self,
            agg_frames: Arc<Vec<data::Frame>>,
            _event_producer: Sender<Event>,
        ) -> Result<(), String> {
            for frame in agg_frames.iter() {
                println!("{} AggFrame! {}", self.x, frame);
            }
            Ok(())
        }
    }

    impl fmt::Display for PseudoPlug {
        fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
            write!(f, "<PseudoPlug x={}>", self.x)
        }
    }

    #[test]
    fn session_loop() {
        let mut sess = Session::new(None);
        sess.register_plugin(PseudoPlug { x: 0 });
        sess.register_plugin(PseudoPlug { x: 1 });
        sess.register_plugin(PseudoPlug { x: 2 });
        assert!(sess.run().is_ok());
    }
}
