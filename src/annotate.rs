use std::sync::{Arc, Mutex};

use opencv::{
	highgui::{self, imshow, WINDOW_GUI_EXPANDED, EVENT_LBUTTONDOWN, resize_window},
	prelude::*,
	Result,imgcodecs,imgproc::{self, LINE_8, convex_hull},
    core::*, types::VectorOfPoint,
//	videoio, imgcodecs::IMREAD_ANYCOLOR,
};
use serde::{Deserialize, Serialize};
use serde_yaml::{self};


#[allow(unused)]

#[derive(Debug, Serialize, Deserialize)]
struct Annotations {
    points : Vec<(i32,i32)>,
    // points : MyPoint2i,
}

// struct MyPoint2i {
//     point : Point2i
// }

// impl std::ops::Deref for MyPoint2i {
//     type Target = Point2i;
//     fn deref(&self) -> &Self::Target {
//         &self.point
//     }
// }

// impl Serialize for MyPoint2i {
//     fn serialize<S>(&self, serializer: S) -> Result<S::Ok, S::Error>
//         where
//             S: serde::Serializer {
        
//     }
// }


#[allow(unused)]
impl Annotations {
    fn init(file_name : String) {

    }
}




pub fn editor(file_name : &String) -> Result<()> { 

    let mut points : Arc<Mutex<Vec<Point2i>>> = Arc::new(Mutex::new(Vec::new()));

    fn save_annotations(annotation_file : &String, points : &Vec<Point2i>) {
    
        let f=std::fs::File::create(annotation_file).expect("Can't open annot file for write");
        let a_points = points.iter().map(|p| (p.x,p.y)).collect();
        let ann : Annotations = Annotations { points: a_points };

        serde_yaml::to_writer(f, &ann).unwrap();
        dbg!("annot written");
    }

    let mut annotation_file = file_name.clone();
    annotation_file.push_str(".annot");

    if let Ok(f) = std::fs::File::open(&annotation_file) {
        let data : Annotations = serde_yaml::from_reader(f).expect("Could not read values.");
        for (x,y) in data.points {
            points.lock().unwrap().push(Point2i::new(x, y));
        }
    } else {
        dbg!("annotation file doesn't exists yet");
    }
//    let f = std::fs::File::open(annotation_file).expect("Could not open file.");

  //  println!("{:?}", scrape_config);


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
            let points2=    points.lock().unwrap();
            let l=points2.len() as i32;
            let s = Size::new(2,l );
            let typ = CV_32S;
            //let mut q = Mat::zeros_size(s, typ)?;
            let mut q = VectorOfPoint::new();
            // let  mut q = Mat::new_rows_cols(points2.len(), 2, CV_32SC2);
            let mut q2 = VectorOfPoint::new();//Mat::default();
            if points2.len() > 0 {
                // q.resize(11);
                for p in points2.iter() {
//                    q.push_back(Point_::new(p.x,p.y));
                    // q2.at_2d(0,0) = 1;
                    q.push(p.clone());
                }
//                let  mut hullpoints=Vec::new();

                convex_hull(&q, &mut q2, true, true);
                for i in 1..points2.len() {
                    let color = Scalar::new(255.,0.,255.,0.);
                    imgproc::line(&mut frame, points2[i-1], points2[i], color,2 , LINE_8, 0)?;
                }
                for i in 0..q2.len() {
                    let color = Scalar::new(0.,255.,0.,0.);
                    let j = if i>0 {i-1} else {q2.len()-1};
                    imgproc::line(&mut frame, q2.get(j)?, q2.get(i)?, color,2 , LINE_8, 0)?;
                }
            }
        }
        imshow(window, &frame)?;
    }
    
    println!("asd");
    Ok(())

}