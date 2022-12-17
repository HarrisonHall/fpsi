use std::sync::Arc;
use std::thread;
use std::vec::Vec;

use crate::data;
use crate::event::Event;
use crate::plugin::Plugin;

use crossbeam_channel::{unbounded, Receiver, Sender};

pub struct Session {
    pub handler: data::Handler,
    plugins: Vec<PluginThreadGroup>,
    // TODO communication
}

impl Session {
    pub fn new() -> Self {
        Session {
            handler: data::Handler::new(),
            plugins: Vec::new(),
        }
    }

    pub fn register_plugin<T: Plugin + 'static>(&mut self, plugin: T) {
        let (producer, consumer) = unbounded::<Event>(); // TODO
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
        let mut exiting: bool = false;
        let mut latest_raw_frames: Vec<data::Frame> = Vec::new();
        let mut latest_agg_frames: Vec<data::Frame> = Vec::new();

        #[derive(PartialEq)]
        enum Step {
            PreAgg,
            PostAgg,
        }
        let mut step = Step::PreAgg;

        // Start event threads
        for plugin_ctx in self.plugins.iter_mut() {
            let plugin = plugin_ctx.plugin.clone();
            let consumer = plugin_ctx.event_consumer.clone();
            let event_producer = self.handler.get_event_producer();
            plugin_ctx.pre_agg_thread = Some(thread::spawn(move || {
                plugin.process_events(consumer, event_producer)
            }));
        }

        while !exiting {
            // Get latest events
            for event in self.handler.get_events().iter_mut() {
                match event {
                    Event::RawData(frame) => {
                        latest_raw_frames.push(frame.clone());
                    }
                    Event::AggData(frame) => {
                        latest_agg_frames.push(frame.clone());
                    }
                    // TODO Send & Recv
                    Event::Die => {
                        exiting = true;
                        self.push_event_to_plugins(event);
                    }
                    _ => {
                        self.push_event_to_plugins(event);
                    }
                };
            }

            // Check if waiting on preags
            match step {
                Step::PreAgg => {
                    if self.waiting_for_preaggs() {
                        // Get agg data from vec
                        let pre_agg_frames = Arc::new(latest_raw_frames.clone());
                        latest_raw_frames.clear();
                        // Start postags
                        for plugin_ctx in self.plugins.iter_mut() {
                            let pre_agg_frames = pre_agg_frames.clone();
                            let plugin = plugin_ctx.plugin.clone();
                            let event_producer = self.handler.get_event_producer();
                            plugin_ctx.post_agg_thread = Some(thread::spawn(move || {
                                plugin.post_agg(pre_agg_frames, event_producer)
                            }));
                        }
                        step = Step::PostAgg;
                    }
                }
                Step::PostAgg => {
                    let time_delta_has_passed: bool = true; // TODO
                    if time_delta_has_passed && !self.waiting_for_postaggs() {
                        // Copy raw data from queue to vec
                        // TODO Normalize timestamps
                        let post_agg_frames = Arc::new(latest_agg_frames.clone());
                        latest_agg_frames.clear();
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
                }
            }
        }

        // Join all threads
        self.join_plugin_threads();

        Ok(())
    }

    fn waiting_for_preaggs(&self) -> bool {
        !self
            .plugins
            .iter()
            .all(|plug| plug.pre_agg_thread_is_done())
    }

    fn waiting_for_postaggs(&self) -> bool {
        !self
            .plugins
            .iter()
            .all(|plug| plug.post_agg_thread_is_done())
    }

    fn push_event_to_plugins(&self, event: &Event) {
        for plugin in self.plugins.iter() {
            match plugin.event_thread {
                Some(_) => {
                    plugin.event_producer.send(event.clone());
                    // TODO - check sending
                }
                None => {}
            }
        }
    }

    fn join_plugin_threads(&mut self) -> () {
        for plugin_ctx in self.plugins.iter_mut() {
            if let Some(event_thread) = plugin_ctx.event_thread.take() {
                match event_thread.join() {
                    Ok(result) => {
                        match result {
                            Ok(()) => {}
                            Err(_err) => {
                                // TODO log error
                            }
                        }
                    }
                    Err(_err) => {
                        // TODO log error
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
    fn event_thread_is_done(&self) -> bool {
        Self::thread_is_done(&self.event_thread)
    }
    fn pre_agg_thread_is_done(&self) -> bool {
        Self::thread_is_done(&self.pre_agg_thread)
    }
    fn post_agg_thread_is_done(&self) -> bool {
        Self::thread_is_done(&self.post_agg_thread)
    }
    fn thread_is_done<T>(thread: &Option<thread::JoinHandle<T>>) -> bool {
        match thread {
            Some(thread) => return thread.is_finished(),
            None => return false,
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    struct PseudoPlug {
        x: u8,
    }
    impl Plugin for PseudoPlug {
        fn load(&mut self) -> Result<(), String> {
            println!("Loaded!");
            Ok(())
        }
        fn process_events(
            &self,
            event_consumer: Receiver<Event>,
            event_producer: Sender<Event>,
        ) -> Result<(), String> {
            loop {
                // Send out new data for current step
                if self.x == 0 {
                    event_producer.send(Event::Die);
                }
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
    }

    #[test]
    fn session_loop() {
        let mut sess = Session::new();
        sess.register_plugin(PseudoPlug { x: 0 });
        sess.register_plugin(PseudoPlug { x: 1 });
        sess.register_plugin(PseudoPlug { x: 2 });
        assert!(sess.run().is_ok());
    }
}
