#[macro_use]
extern crate approx;

use opencv::{
	highgui,
	prelude::*,
	Result,
	videoio,
};

use clap::Parser;
use clap::*;
mod annotate;
mod montage;
mod glue;
mod poly_distance;

#[derive(Copy, Clone, PartialEq, Eq, PartialOrd, Ord, ValueEnum,Debug)]
enum Mode {
	Annotate,
	Montage,
}

#[derive(Parser, Debug)]
#[command(author, version, about, long_about = None)]
struct Args {
	#[arg(value_enum)]
	mode : Mode,
	images: Vec<String>,
}

fn main() -> Result<()> {
	let args = Args::parse();

	match args.mode {
		Mode::Annotate => 	for image in &args.images {
			annotate::editor(image)?;
		}
		Mode::Montage => {
			montage::editor(&args.images)?;
		}
	}

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


