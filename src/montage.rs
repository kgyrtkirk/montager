use std::sync::{Arc, Mutex};

use opencv::{
	highgui::{self, imshow, WINDOW_GUI_EXPANDED, EVENT_LBUTTONDOWN, resize_window},
	prelude::*,
	Result,
    imgcodecs,
    imgproc::{self, LINE_8, convex_hull},
    core::*, types::VectorOfPoint,
};


struct MontageImage {
    file_name: String,
    image: Mat,
    position: Point2i,

}

struct MontageEditor {
    points : Arc<Mutex<VectorOfPoint>>,// = Arc::new(Mutex::new(VectorOfPoint::new()));
    annotation_file: String,
    #[allow(unused)]
    file_name: String,
    image: Mat,
    
}

#[allow(unused)]
impl MontageEditor {
    // fn new(file_name : &String) -> MontageEditor {

    // }
    // fn draw(self : &MontageEditor) -> Result<Mat> {

    //     Ok(()))
    // }
    // fn add_point(self : &MontageEditor, point : Point2i) {
    //     self.points.lock().unwrap().push(point);
    // }
}


pub fn editor(file_name : &Vec<String>) -> Result<()> { 

    let window = "montage-gen montage";

    highgui::named_window(window, highgui::WINDOW_KEEPRATIO | WINDOW_GUI_EXPANDED)?;

    highgui::set_mouse_callback(window, Some(Box::new({
    //    let annotation_editor = Arc::clone(&annotation_editor);
        move |event, x ,y, _flags| {
            println!(" {} {} ",x,y);
            match event {
                EVENT_LBUTTONDOWN => {
                    println!(" {} {} ",x,y);
                    // annotation_editor.lock().unwrap().add_point(Point2i::new(x,y));
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
        // let annotation_editor = annotation_editor.lock().unwrap();
        // if key == 's' as i32 {
        //     annotation_editor.save();
        //     break;
        // }
        // let frame=annotation_editor.draw()?;
        // imshow(window, &frame)?;
    }
    Ok(())
}