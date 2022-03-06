
-- Check if table exists
SELECT name FROM sqlite_master WHERE name ='dataframes' and type='table';

-- Create dataframes table
CREATE TABLE dataframes(
			 df_id INTEGER PRIMARY KEY AUTOINCREMENT,
			 df_source TEXT,
			 df_type TEXT,
			 df_data BLOB,
			 df_time TEXT
);

-- Insert dataframe
INSERT INTO dataframes(df_source, df_type, df_data, df_time)
VALUES('source', 'type', null, 'time');
INSERT INTO dataframes(df_source, df_type, df_data, df_time)
VALUES('counter', 'agg', 'test', '1971-12-19 07:42');

-- Get last packet id
SELECT last_insert_rowid();

-- Get most recent packets (oldest first)
SELECT * FROM
(SELECT * FROM dataframes ORDER BY df_id DESC LIMIT 10)
ORDER BY df_id ASC;
