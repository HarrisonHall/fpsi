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
    secs_per_tick: f32,
    max_channel_size: usize,
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
            secs_per_tick: 0.5,
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
        let (prod_sender, prod_receiver) =
            event::bounded::<event::Event<So, Fr, St>>(self.max_channel_size);
        let (agg_sender, agg_receiver) =
            event::bounded::<event::Event<So, Fr, St>>(self.max_channel_size);
        let (cons_sender, cons_receiver) =
            event::bounded::<event::Event<So, Fr, St>>(self.max_channel_size);

        // Start produce threads
        let mut produce_threads = Vec::new();
        for handler in self.event_handlers.iter() {
            let handler = handler.clone();
            let ctx = event::HandlerContext::new(event_sender.clone(), prod_receiver.clone());
            produce_threads.push(thread::spawn(move || {
                handler.produce(ctx);
            }));
        }

        // Start aggregrate threads
        let mut aggregate_threads = Vec::new();
        for handler in self.event_handlers.iter() {
            let handler = handler.clone();
            let ctx = event::HandlerContext::new(event_sender.clone(), agg_receiver.clone());
            aggregate_threads.push(thread::spawn(move || {
                handler.aggregate(ctx);
            }));
        }

        // Start consume threads
        let mut consume_threads = Vec::new();
        for handler in self.event_handlers.iter() {
            let handler = handler.clone();
            let ctx = event::HandlerContext::new(event_sender.clone(), cons_receiver.clone());
            consume_threads.push(thread::spawn(move || {
                handler.consume(ctx);
            }));
        }

        // Handle ticks & Aggregates
        let mut raws: Vec<Fr> = Vec::new();
        let mut last_agg = std::time::Instant::now();
        'event_handler: loop {
            // Process new event
            if let Ok(event) = event_receiver.try_recv() {
                // Forward events
                prod_sender.send(event.clone()).ok();
                agg_sender.send(event.clone()).ok();
                cons_sender.send(event.clone()).ok();
                // Handle shutdown
                match event {
                    event::Event::Shutdown => {
                        break 'event_handler;
                    }
                    event::Event::Raw(raw) => {
                        raws.push(raw);
                    }
                    _ => {}
                };
            }
            // Handle aggregates
            if last_agg.elapsed().as_secs_f32() >= self.secs_per_tick {
                println!("Aggregating!");
                last_agg = std::time::Instant::now();
                let raws_to_agg = Arc::new(raws.clone());
                agg_sender.send(event::Event::Raws(raws_to_agg)).ok();
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
