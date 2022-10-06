use std::sync::{Arc, Mutex};

use opencv::{
	highgui::{self, imshow, WINDOW_GUI_EXPANDED, EVENT_LBUTTONDOWN, resize_window},
	prelude::*,
	Result,
    imgcodecs,
    imgproc::{self, LINE_8, convex_hull},
    core::*, types::VectorOfPoint,
};
use serde::{Deserialize, Serialize};
use serde_yaml::{self};

#[allow(unused)]
#[derive(Debug, Serialize, Deserialize)]
struct OnDiskAnnotations {
    points : Vec<(i32,i32)>,
}

#[allow(unused)]
impl OnDiskAnnotations {
    fn init(file_name : String) {

    }
}

pub fn editor(file_name : &String) -> Result<()> { 

    let mut points = Arc::new(Mutex::new(VectorOfPoint::new()));

    fn save_annotations(annotation_file : &String, points : &Vector<Point2i>) {
    
        let f=std::fs::File::create(annotation_file).expect("Can't open annot file for write");
        let a_points = points.iter().map(|p| (p.x,p.y)).collect();
        let ann : OnDiskAnnotations = OnDiskAnnotations { points: a_points };

        serde_yaml::to_writer(f, &ann).unwrap();
        dbg!("annot written");
    }

    let mut annotation_file = file_name.clone();
    annotation_file.push_str(".annot");

    if let Ok(f) = std::fs::File::open(&annotation_file) {
        let data : OnDiskAnnotations = serde_yaml::from_reader(f).expect("Could not read values.");
        for (x,y) in data.points {
            points.lock().unwrap().push(Point2i::new(x, y));
        }
    } else {
        dbg!("annotation file doesn't exists yet");
    }

    let window = "montage-gen annotate";
    highgui::named_window(window, highgui::WINDOW_KEEPRATIO | WINDOW_GUI_EXPANDED)?;
    highgui::set_mouse_callback(window, Some(Box::new({
        let points = Arc::clone(&points);
        move |event, x ,y, _flags| {
            match event {
                EVENT_LBUTTONDOWN => {
                    println!(" {} {} ",x,y);
                    points.lock().unwrap().push(Point2i::new(x,y));
                }
                _ => {

                }
            }
        }
    })))?;

    let mut image=imgcodecs::imread(&file_name, 1).unwrap();
    // FIXME: missing image not detected

    println!("asd {}", image.size().unwrap().width);


    resize_window(window, 500, 500)?;
    while let key = highgui::wait_key(100)? {
        if key==27 {
            break;
        }
        if key == 's' as i32 {
            save_annotations(&annotation_file, &points.lock().unwrap());
            break;
        }
        let mut frame=image.clone();
        {
            let points2=    points.lock().unwrap().to_owned();
            let mut hull_points = VectorOfPoint::new();//Mat::default();
            if points2.len() > 0 {

                convex_hull(&points2, &mut hull_points, true, true);
                
                for i in 1..points2.len() {
                    let color = Scalar::new(255.,0.,255.,0.);
                    imgproc::line(&mut frame, points2.get(i-1)?, points2.get(i)?, color,2 , LINE_8, 0)?;
                }
                for i in 0..hull_points.len() {
                    let color = Scalar::new(0.,255.,0.,0.);
                    let j = if i>0 {i-1} else {hull_points.len()-1};
                    imgproc::line(&mut frame, hull_points.get(j)?, hull_points.get(i)?, color,2 , LINE_8, 0)?;
                }
            }
        }
        imshow(window, &frame)?;
    }
    
    Ok(())

}