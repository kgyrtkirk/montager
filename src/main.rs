use opencv::{
	highgui,
	prelude::*,
	Result,
	videoio,
};

use clap::Parser;
mod annotate;

#[derive(Parser, Debug)]
#[command(author, version, about, long_about = None)]
struct Args {
   #[arg(short, long)]
   image: String,
}

fn main() -> Result<()> {
	let args = Args::parse();

 	annotate::editor(&args.image)?;
	Ok(())
}

#[allow(unused)]
fn main1() -> Result<()> {
		let args: Vec<String> = std::env::args().collect();
    dbg!(args);

	let window = "video capture";
	highgui::named_window(window, highgui::WINDOW_AUTOSIZE)?;
	opencv::opencv_branch_32! {
		let mut cam = videoio::VideoCapture::new_default(0)?; // 0 is the default camera
	}
	opencv::not_opencv_branch_32! {
		let mut cam = videoio::VideoCapture::new(0, videoio::CAP_ANY)?; // 0 is the default camera
	}
	let opened = videoio::VideoCapture::is_opened(&cam)?;
	if !opened {
		panic!("Unable to open default camera!");
	}
	loop {
		let mut frame = Mat::default();
		cam.read(&mut frame)?;
		if frame.size()?.width > 0 {
			highgui::imshow(window, &mut frame)?;
		}
		let key = highgui::wait_key(10)?;
		if key > 0 && key != 255 {
			break;
		}
	}
	Ok(())
}

