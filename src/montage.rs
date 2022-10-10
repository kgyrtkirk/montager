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

    fn new(file_name: &String,pos:&Point2i) -> MontageImage {
        let image = imgcodecs::imread(&file_name, 1).unwrap();
        // image.convert_to(m, rtype, alpha, beta)
        // imgcodecs::conv
        if image.size().unwrap().width<=0 {
            panic!("Can't open file"); // FIXME: show filename
        }

        MontageImage {
            file_name: file_name.clone(),
            position: pos.clone(),
            image: image.clone(),
            image2: Mat::default(),
        }
    }
    fn render(&mut self, size : Size) -> Result<()>{
        self.image2 = Mat::zeros_size(size, CV_8UC3).unwrap().to_mat().unwrap();
        // //let size = self.image.size().unwrap();
        // // self.image2.set_rows(size.width);
        let m = imgproc::get_rotation_matrix_2d(Point2f::new(50.0, 50.0), 10.0, 1.0).unwrap();

        // let size= Size ::new(500,500);
        // println!("s: {:?}", size);
        imgproc::warp_affine(
            &self.image,
            &mut self.image2,
            &m,
            size,
            INTER_LINEAR,
            BORDER_CONSTANT,
            Scalar::new(0.0, 0.0, 0.0, 0.0),
        );
        // println!("s: {:?}", self.image2.size());
        // self.image.copy_to(&mut self.image2);
        let pt1 = Point2i::new(0, 0);
        let pt2 = self.position;
        let pt3 = Point2i::new(300, 150);
        let color = Scalar::new(255., 255., 255., 255.);
        imgproc::line(&mut self.image2, pt1, pt2, color, 8, LINE_8, 1);
        imgproc::line(&mut self.image2, pt2, pt3, color, 8, LINE_8, 1);

        Ok({})
    }

    fn dist(self : &MontageImage,p:&Point2i) -> f64{
        let d=(self.position-*p).norm();
        d
    }
    fn sample(self : &MontageImage,p:&Point2i) -> Result<&Vec3b>{
        let q=self.image2.at_2d::<Vec3b>(p.x,p.y)?;
        Ok(q)
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
            width: 500  ,
            height: 500,
        };
        // let m = MontageImage::new(&String::from("r.png"));
        let mut image = Mat::zeros_size(size, CV_8UC3).unwrap().to_mat().unwrap();
        let color = Scalar::new(255., 0., 255., 0.);
        let pt1 = Point2i::new(100, 100);
        let pt2 = Point2i::new(200, 50);
        let pt3 = Point2i::new(300, 150);
        imgproc::line(&mut image, pt1, pt2, color, 8, LINE_8, 1);
        let images = vec![
            MontageImage::new(&String::from("r2.png"),&Point2i::new(300, 100)),
            MontageImage::new(&String::from("r3.png"),&Point2i::new(100, 200)),
            ];
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

        self.image = Mat::new_size_with_default(self.size, CV_8UC3,  Scalar::from(127))?;

        println!("ok?");
        let v0 = VecN::new(1,2,3,4);
        let v = Vec3b::from([255,0,255]);
        for row in 0..self.size.height {
        for col in 0..self.size.width {
                let p : Point2i =Point_ { x: row, y: col };
                let mut dist_color : Vec<(f64,&Vec3b)> = self.images.iter().map( |i| (i.dist(&p),i.sample(&p).unwrap()) ).collect();

                    let a = 0.;
                    // cmp::
                    // 1.cmp(2);
                    // 1.0.cmp
                    // float_cmp
                dist_color.sort_by( |(d,v),(d2,v2)| d.total_cmp(d2));


                // println!("{:?}",dist_color);


                // for img in self.images.iter_mut() {

                // }

                // let q=self.image.at_2d::<Vec3b>(row-1000, col)?;
                let q=dist_color.get(0).unwrap().1;
                //self.image[1,2];
                *self.image.at_2d_mut(row, col)?=*q;
        
            }
        }
        println!("ok");
        println!("ok");

        if(false) {
        for i in self.images.iter_mut() {
            // i.render();
            let image = &i.image;
            let size = image.size().unwrap();
            let roi = Rect {
                x: i.position.x,
                y: i.position.y,
                width: size.width,
                height: size.height,
            };
            
            let mut dest = Mat::roi(&mut self.image, roi)?;
            println!("{:?}",dest);
            println!("{:?}",image);
  //           i.image2.copy_to(&mut dest)?;
             image.copy_to(&mut dest)?;
            // normalize(src, dst, alpha, beta, norm_type, dtype, mask)
            // panic!("asd");
            // self.image.re
        }
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
        editor.montage.render()?;
        imshow(window, &editor.montage.image)?;
//        imshow(window, &editor.montage.images.get(0).unwrap().image2)?;
//        imshow(window, &editor.montage.image)?;
        // if key == 's' as i32 {
        //     annotation_editor.save();
        //     break;
        // }
        // let frame=annotation_editor.draw()?;
        // imshow(window, &frame)?;
    }
    Ok(())
}
