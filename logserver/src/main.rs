use axum::{Router, routing::get_service};
use mime::TEXT_PLAIN;
use std::{env::var, fs::OpenOptions, io::Write};
use time_format::format_iso8601_utc;
use tower_http::services::ServeFile;

#[tokio::main]
async fn main() {
    let router = Router::new().route(
        "/log",
        get_service(ServeFile::new_with_mime(log_path(), &TEXT_PLAIN)).post(log),
    );

    let listener = tokio::net::TcpListener::bind("0.0.0.0:4000").await.unwrap();
    axum::serve(listener, router).await.unwrap();
}

fn log_path() -> String {
    var("MUD_LOG_PATH").unwrap_or_else(|_| "./log".to_string())
}

async fn log(data: String) {
    let mut file = OpenOptions::new()
        .append(true)
        .create(true)
        .open(log_path())
        .expect("could not open log file");

    let time = format_iso8601_utc(time_format::now().unwrap()).unwrap();
    let line = format!("{time}\t{data}\n");

    file.write_all(line.as_bytes())
        .expect("could not append to log file");
}
