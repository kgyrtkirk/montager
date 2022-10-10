use std::sync::{Arc, Mutex};

#[allow(unused)]
use opencv::{
    core::*,
    highgui::{self, imshow, resize_window, EVENT_LBUTTONDOWN, WINDOW_GUI_EXPANDED},
    imgcodecs,
    imgproc::{self,  INTER_LINEAR, LINE_8},
    prelude::*,
    types::VectorOfPoint,
    Result,
};

#[allow(unused)]
struct MontageImage {
    file_name: String,
    position: Point2i,
    image: Mat,
    image2: Mat,
}

#[allow(unused)]
impl MontageImage {
    fn new(file_name: &String) -> MontageImage {
        let image = imgcodecs::imread(&file_name, 1).unwrap();
        if image.size().unwrap().width<=0 {
            panic!("Can't open file"); // FIXME: show filename
        }

        MontageImage {
            file_name: file_name.clone(),
            position: Point2i::new(1, 2),
            image: image.clone(),
            image2: image.clone(),
        }
    }
    fn render(&mut self, size : Size) -> Result<()>{
        self.image2 = Mat::zeros_size(size, CV_8UC3).unwrap().to_mat().unwrap();
        //let size = self.image.size().unwrap();
        // self.image2.set_rows(size.width);
        let m = imgproc::get_rotation_matrix_2d(Point2f::new(100.0, 100.0), 10.0, 1.0).unwrap();

        let size= Size ::new(200,200);
        println!("s: {:?}", size);
        imgproc::warp_affine(
            &self.image,
            &mut self.image2,
            &m,
            size,
            INTER_LINEAR,
            BORDER_CONSTANT,
            Scalar::new(0.0, 0.0, 0.0, 0.0),
        );
        Ok({})
    }
}

#[allow(unused)]
struct Montage {
    image: Mat,
    images: Vec<MontageImage>,
    size: Size2i,
}

#[allow(unused)]
impl Montage {
    fn new(file_name: &String) -> Montage {
        let size = Size_ {
            width: 600  ,
            height: 600,
        };
        let m = MontageImage::new(&String::from("r.png"));
        let mut image = Mat::zeros_size(size, CV_8UC3).unwrap().to_mat().unwrap();
        let color = Scalar::new(255., 0., 255., 0.);
        let pt1 = Point2i::new(100, 100);
        let pt2 = Point2i::new(200, 50);
        let pt3 = Point2i::new(300, 150);
        imgproc::line(&mut image, pt1, pt2, color, 8, LINE_8, 1);
        let images = vec![(m)];
        // let images = vec![Box::new(m)];
        let mut m = Montage {
            image: image,
            images: images,
            size: size,
        };
//        Montage::render(&mut m);
        m.render();
        imgproc::line(&mut m.image, pt1, pt3, color, 8, LINE_8, 1);
        m
    }
    fn render(&mut self) -> Result<()>{
        
        self.images.iter_mut().for_each( |m| m.render(self.size).unwrap() );

        for i in self.images.iter_mut() {
            // i.render();
            let image = &i.image2;
            let size = image.size().unwrap();
            let roi = Rect {
                x: 100,
                y: 100,
                width: size.width,
                height: size.height,
            };
            
            let mut dest = Mat::roi(&mut self.image, roi)?;
            println!("{:?}",dest);
            println!("{:?}",image);
  //           i.image2.copy_to(&mut dest)?;
            image.copy_to(&mut self.image)?;
            // normalize(src, dst, alpha, beta, norm_type, dtype, mask)
            // panic!("asd");
            // self.image.re
        }
        Ok({})
    }
}

struct MontageEditor {
    montage: Montage,
    //    active_image : i32,
}

#[allow(unused)]
impl MontageEditor {
    fn new(file_name: &String) -> MontageEditor {
        let montage = Montage::new(file_name);
        MontageEditor { montage: montage }
    }
    // fn draw(self : &MontageEditor) -> Result<Mat> {

    //     Ok(()))
    // }
    // fn add_point(self : &MontageEditor, point : Point2i) {
    //     self.points.lock().unwrap().push(point);
    // }
}

pub fn editor(file_name: &Vec<String>) -> Result<()> {
    let montage: Arc<Mutex<MontageEditor>> =
        Arc::new(Mutex::new(MontageEditor::new(file_name.get(0).unwrap())));

    let window = "montage-gen montage";

    highgui::named_window(window, highgui::WINDOW_KEEPRATIO | WINDOW_GUI_EXPANDED)?;

    highgui::set_mouse_callback(
        window,
        Some(Box::new({
            //    let annotation_editor = Arc::clone(&annotation_editor);
            move |event, x, y, _flags| {
                println!(" {} {} ", x, y);
                match event {
                    EVENT_LBUTTONDOWN => {
                        println!(" {} {} ", x, y);
                        // annotation_editor.lock().unwrap().add_point(Point2i::new(x,y));
                    }
                    _ => {}
                }
            }
        })),
    )?;

    resize_window(window, 500, 500)?;
    loop {
        let key = highgui::wait_key(100)?;
        if key == 27 {
            break;
        }
        let mut editor = montage.lock().unwrap();
        editor.montage.render();
        imshow(window, &editor.montage.image)?;
        // if key == 's' as i32 {
        //     annotation_editor.save();
        //     break;
        // }
        // let frame=annotation_editor.draw()?;
        // imshow(window, &frame)?;
    }
    Ok(())
}
