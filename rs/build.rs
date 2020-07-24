use std::env::var;
use std::process::Command;
use std::path::{Path};

fn main() {
    let manifest_dir = var("CARGO_MANIFEST_DIR").unwrap();
    // let tqapi_build_dir = Path::new(&manifest_dir).join("../build/dist");

    if true {
        Command::new("bash")
            .args(&["./build.sh"])
            .current_dir(&manifest_dir)
            .status()
            .unwrap_or_else(|e| {
                panic!("Failed to build tqapi: {}", e);
            });
    }

    println!("cargo:rustc-link-lib={}", "stdc++");
    println!("cargo:rustc-link-lib={}", "myutils");
    println!("cargo:rustc-link-lib={}", "msgpack");
    println!("cargo:rustc-link-lib={}", "jsoncpp");
    println!("cargo:rustc-link-lib={}", "snappy");
    println!("cargo:rustc-link-search={}/../build/dist/cpp", manifest_dir);
    println!("cargo:rustc-link-search={}/../build/dist/bin", manifest_dir);
    //println!("cargo:rustc-link-search={}/libraries/usr/lib/arm-linux-gnueabihf", manifest_dir);
    println!("cargo:rustc-link-search=native=lib");
}
