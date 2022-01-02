pub struct DataFrame {
    // TODO - json
    pub id: u64,
    pub source: String,
    pub data_type: String,
    // TODO - time
    // TODO - data lock mutex?
}

impl DataFrame {
    pub fn new() -> Self {
        //static mut id_count: u64 = 0;
        let d: DataFrame = DataFrame {
            id: 0,
            source: String::from("Test"),
            data_type: String::from("Foobar"),
        };

        //id_count = id_count + 1;
        return d;
    }

    pub fn get_id(&self) -> &u64 {
        return &self.id; // Return immutable reference
    }

    pub fn get_source(&self) -> &String {
        return &self.source; // Return immutable reference
    }

    pub fn get_data_type(&self) -> &String {
        return &self.data_type; // Return immutable reference
    }

    pub fn set_id(&mut self, id: u64) {
        self.id = id;
    }

    pub fn set_source(&mut self, source: String) {
        self.source = source.clone();
    }

    pub fn set_data_type(&mut self, data_type: String) {
        self.data_type = data_type.clone();
    }
}

impl Drop for DataFrame {
    fn drop(&mut self) {
        println!("Dropping packet with id {}", self.get_id());
    }
}

impl Clone for DataFrame {
    fn clone(&self) -> Self {
        let d: DataFrame = DataFrame {
            id: self.get_id().clone(),
            source: self.get_source().clone(),
            data_type: self.get_data_type().clone(),
        };
        return d;
    }
}
