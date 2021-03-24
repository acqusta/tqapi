use std::env::var;
#[cfg(target_os = "linux")]
use std::process::Command;
//use std::path::{Path};

fn main() {
    let manifest_dir = var("CARGO_MANIFEST_DIR").unwrap();

    setup(&manifest_dir);    

    // println!("cargo:rustc-link-search={}/../build-win32/dist/cpp", manifest_dir);
    // println!("cargo:rustc-link-search={}/../build-win32/dist/bin", manifest_dir);
    // println!("cargo:rustc-link-lib={}", "stdc++");

    println!("cargo:rustc-link-lib={}", "myutils");
    println!("cargo:rustc-link-lib={}", "msgpack");
    println!("cargo:rustc-link-lib={}", "jsoncpp");
    println!("cargo:rustc-link-lib={}", "snappy");
    println!("cargo:rustc-link-lib={}", "tqapi-static");
}

// #[cfg(all(target_os = "windows"))]//, target_env="gnu"))]
// fn setup(manifest_dir: &str) {
//     println!("cargo:rustc-link-search={}/../build-win32/dist/cpp", manifest_dir);
//     println!("cargo:rustc-link-search={}/../build-win32/dist/bin", manifest_dir);
//     assert!(false);
// }

#[cfg(target_os = "linux")]
fn setup(manifest_dir: &str) {
    Command::new("bash")
    .args(&["./build.sh"])
    .current_dir(&manifest_dir)
    .status()
    .unwrap_or_else(|e| {
        panic!("Failed to build tqapi: {}", e);
    });

    println!("cargo:rustc-link-search={}/../build-linux/dist/cpp", manifest_dir);
    println!("cargo:rustc-link-search={}/../build-linux/dist/bin", manifest_dir);

    println!("cargo:rustc-link-lib={}", "stdc++");
}

#[cfg(target_os = "macos")]
fn setup(manifest_dir: &str) {
    use std::process::Command;
    //use std::path::{Path};
    Command::new("bash")
    .args(&["./build.sh"])
    .current_dir(&manifest_dir)
    .status()
    .unwrap_or_else(|e| {
        panic!("Failed to build tqapi: {}", e);
    });

    println!("cargo:rustc-link-search={}/../build-linux/dist/cpp", manifest_dir);
    println!("cargo:rustc-link-search={}/../build-linux/dist/bin", manifest_dir);

    println!("cargo:rustc-link-lib={}", "c++");
    println!("cargo:rustc-link-lib={}", "iconv");
}

#[cfg(target_os = "windows")]
fn setup(manifest_dir: &str) {
    println!("cargo:rustc-link-search={}/../build-win32/dist/cpp", manifest_dir);
    println!("cargo:rustc-link-search={}/../build-win32/dist/bin", manifest_dir);
}