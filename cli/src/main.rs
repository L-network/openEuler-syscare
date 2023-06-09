use std::process::exit;

use log::error;

use syscare::cli::*;

fn main() {
    match SyscareCLI::run() {
        Ok(exit_code) => {
            exit(exit_code);
        },
        Err(e) => {
            error!("{}", e);
            exit(-1);
        }
    }
}
