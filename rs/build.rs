use std::env::var;

fn main() {
    let manifest_dir = var("CARGO_MANIFEST_DIR").unwrap();
    println!("cargo:rustc-link-search={}/../build/dist/cpp", manifest_dir);
    println!("cargo:rustc-link-search={}/../build/dist/bin", manifest_dir);
    //println!("cargo:rustc-link-search={}/libraries/usr/lib/arm-linux-gnueabihf", manifest_dir);
    println!("cargo:rustc-link-search=native=lib");
}
