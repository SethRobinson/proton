extern crate escapi;

/* "simplest", example of simply enumerating the available devices with ESCAPI */

fn main() {
    println!("devices: {}", escapi::num_devices());

    /* Set up capture parameters.
    * ESCAPI will scale the data received from the camera
    * (with point sampling) to whatever values you want.
    * Typically the native resolution is 320*240.
    */
    const W: u32 = 320;
    const H: u32 = 240;

    let mut camera = escapi::init(0, W, H, 15).expect("Could not initialize the camera");
    println!("capture initialized, device name: {}", camera.name());

    for i in 0..15 {
        let pixels = camera.capture().expect("Could not capture an image");
        println!("Frame #{}, captured {} bytes", i, pixels.len());
    }

    println!("shutting down");
}
