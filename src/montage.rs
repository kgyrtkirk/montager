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
    position: Point2i,
    image: Mat,
}

impl MontageImage {
    fn new(file_name : &String) -> MontageImage {
        let image = imgcodecs::imread(&file_name, 1).unwrap();
        if !image.is_allocated() {
            panic!("Can't open file");  // FIXME: show filename
        }

        MontageImage { 
            file_name: file_name.clone(),
             position: Point2i::new(1,2), 
             image: image
        }
    }
}

struct Montage {
    image: Mat,
    images : Vec<MontageImage>,
    size : Size2i,
}

impl Montage {
    fn new(file_name : &String) -> Montage {
        let size=Size_ { width: 1280, height: 1024 };
        let images = vec![MontageImage::new(String::from("r.png"))];
        Montage { image: Mat::new_size(size, typ), images: images, size:size }
    }
    
}


struct MontageEditor {
    montage : Montage,
//    active_image : i32,
}

#[allow(unused)]
impl MontageEditor {
    fn new(file_name : &String) -> MontageEditor {
        let montage = Montage::new(file_name);
        MontageEditor { montage: montage,  }
    }
    // fn draw(self : &MontageEditor) -> Result<Mat> {

    //     Ok(()))
    // }
    // fn add_point(self : &MontageEditor, point : Point2i) {
    //     self.points.lock().unwrap().push(point);
    // }
}


pub fn editor(file_name : &Vec<String>) -> Result<()> { 
    let montage : Arc<Mutex<MontageEditor>>=Arc::new(Mutex::new(MontageEditor::new(file_name.get(0))));


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