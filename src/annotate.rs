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

pub struct AnnotationEditor {
    points : Arc<Mutex<VectorOfPoint>>,// = Arc::new(Mutex::new(VectorOfPoint::new()));
    annotation_file: String,
    #[allow(unused)]
    file_name: String,
    pub image: Mat,
}

#[allow(unused)]
impl AnnotationEditor {
    pub fn new(file_name : &String) -> AnnotationEditor {

        let mut image=imgcodecs::imread(&file_name, 1).unwrap();
        // FIXME: missing image not detected
    
        println!("asd {}", image.size().unwrap().width);
 
        let mut points = Arc::new(Mutex::new(VectorOfPoint::new()));

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

        AnnotationEditor {
            points: points,
            annotation_file: annotation_file,
            file_name: file_name.clone(),
            image: image,
        }
    }
    fn save(self : &AnnotationEditor) {
        let f=std::fs::File::create(&self.annotation_file).expect("Can't open annot file for write");
        let points = self.points.lock().unwrap();
        let a_points = points.iter().map(|p| (p.x,p.y)).collect();
        let ann : OnDiskAnnotations = OnDiskAnnotations { points: a_points };

        serde_yaml::to_writer(f, &ann).unwrap();
        dbg!("annot written");
    }
    fn draw(self : &AnnotationEditor) -> Result<Mat> {

        let mut frame=self.image.clone();
        let points2 = self.points.lock().unwrap().to_owned();
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
        Ok(frame)
    }
    fn add_point(self : &AnnotationEditor, point : Point2i) {
        self.points.lock().unwrap().push(point);
    }
}


pub fn editor(file_name : &String) -> Result<()> { 

    let annotation_editor = Arc::new(Mutex::new(AnnotationEditor::new(file_name)));
    let window = "montage-gen annotate";

    highgui::named_window(window, highgui::WINDOW_KEEPRATIO | WINDOW_GUI_EXPANDED)?;
    highgui::set_mouse_callback(window, Some(Box::new({
       let annotation_editor = Arc::clone(&annotation_editor);
        move |event, x ,y, _flags| {
            match event {
                EVENT_LBUTTONDOWN => {
                    println!(" {} {} ",x,y);
                    annotation_editor.lock().unwrap().add_point(Point2i::new(x,y));
                }
                _ => {
                }
            }
        }
    })))?;

    resize_window(window, 500, 500)?;
    loop {
        let key = highgui::wait_key(100)?;
        if key==27 {
            break;
        }
        let annotation_editor = annotation_editor.lock().unwrap();
        if key == 's' as i32 {
            annotation_editor.save();
            break;
        }
        let frame=annotation_editor.draw()?;
        imshow(window, &frame)?;
    }
    Ok(())
}