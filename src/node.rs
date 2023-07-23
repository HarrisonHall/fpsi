use std::sync::Arc;
use std::sync::RwLock;
use std::thread;
use std::vec::Vec;

use crate::data;
use crate::event;

pub struct Node<So, Fr, St>
where
    So: data::Source,
    Fr: data::Frame<So>,
    St: data::State,
{
    event_handlers: Vec<Arc<dyn event::Handler<So, Fr, St>>>,
    sec_per_tick: f32,
    max_channel_size: usize,
    // config: Config,
    // handler: handler::Handler,
    // communicator: CommController,
    // plugins: Vec<PluginThreadGroup>,
}

impl<So, Fr, St> Node<So, Fr, St>
where
    So: data::Source,
    Fr: data::Frame<So>,
    St: data::State,
{
    pub fn new() -> Self {
        Self {
            event_handlers: Vec::new(),
            sec_per_tick: 1.0,
            max_channel_size: 1024,
        }
    }

    pub fn register_event_handler(&mut self, handler: Arc<dyn event::Handler<So, Fr, St>>) {
        self.event_handlers.push(handler);
    }

    pub fn run<'r>(&mut self) -> Result<(), Box<dyn std::error::Error>> {
        // Initialize event handlers
        let (event_sender, event_receiver) =
            event::bounded::<event::Event<So, Fr, St>>(self.max_channel_size);
        let (state_sender, state_receiver) = event::bounded::<St>(self.max_channel_size);
        let (raw_sender, raw_receiver) =
            event::bounded::<data::FrameVec<Fr>>(self.max_channel_size);
        let (agg_sender, agg_receiver) = event::bounded::<Fr>(self.max_channel_size);
        let shutdown = Arc::new(RwLock::new(false));

        // Start produce threads
        let mut produce_threads = Vec::new();
        for handler in self.event_handlers.iter() {
            let handler = handler.clone();
            let ctx = event::HandlerContext::new(
                event_sender.clone(),
                state_receiver.clone(),
                None,
                None,
                shutdown.clone(),
            );
            produce_threads.push(thread::spawn(move || {
                handler.produce(ctx);
            }));
        }

        // Start aggregrate threads
        let mut aggregate_threads = Vec::new();
        for handler in self.event_handlers.iter() {
            let handler = handler.clone();
            let ctx = event::HandlerContext::new(
                event_sender.clone(),
                state_receiver.clone(),
                Some(raw_receiver.clone()),
                None,
                shutdown.clone(),
            );
            aggregate_threads.push(thread::spawn(move || {
                handler.aggregate(ctx);
            }));
        }

        // Start consume threads
        let mut consume_threads = Vec::new();
        for handler in self.event_handlers.iter() {
            let handler = handler.clone();
            let ctx = event::HandlerContext::new(
                event_sender.clone(),
                state_receiver.clone(),
                None,
                Some(agg_receiver.clone()),
                shutdown.clone(),
            );
            consume_threads.push(thread::spawn(move || {
                handler.consume(ctx);
            }));
        }

        // Handle ticks & Aggregates
        let mut raws: Vec<Arc<Fr>> = Vec::new();
        let mut last_agg = std::time::Instant::now();
        'event_handler: loop {
            // Process new event
            if let Ok(event) = event_receiver.try_recv() {
                match event {
                    event::Event::<So, Fr, St>::State(state) => {
                        state_sender.send(state).ok();
                    }
                    event::Event::<So, Fr, St>::Raw(raw) => {
                        raws.push(Arc::new(raw));
                    }
                    event::Event::<So, Fr, St>::Agg(agg) => {
                        agg_sender.send(agg).ok();
                    }
                    event::Event::<So, Fr, St>::Shutdown => {
                        println!("Got shutdown!");
                        if let Ok(mut shutdown) = shutdown.write() {
                            *shutdown = true;
                            break 'event_handler;
                        }
                    }
                    _ => {}
                };
            }
            // Handle aggregates
            if last_agg.elapsed().as_secs_f32() >= self.sec_per_tick {
                println!("Aggregating!");
                last_agg = std::time::Instant::now();
                let raws_to_agg = Arc::new(raws.clone());
                raw_sender.send(raws_to_agg).ok();
                raws.clear();
            }
        }

        // Join all threads
        for thread in produce_threads {
            thread.join().ok();
        }
        for thread in aggregate_threads {
            thread.join().ok();
        }
        for thread in consume_threads {
            thread.join().ok();
        }

        Ok(())
    }
}
