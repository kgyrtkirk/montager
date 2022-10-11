use std::sync::{Arc, Mutex};

#[allow(unused)]
use opencv::{
    core::*,
    highgui::{self, imshow, resize_window, EVENT_LBUTTONDOWN, WINDOW_GUI_EXPANDED},
    imgcodecs,
    imgproc::{self, INTER_LINEAR, LINE_8},
    prelude::*,
    types::VectorOfPoint,
    Result,
};

use crate::annotate::AnnotationEditor;

#[allow(unused)]
struct MontageImage {
    aimage: AnnotationEditor,
    position: Point2i,
    image2: Mat,
    dist_map: Mat,
}

mod Mat2dOps {
    use opencv::{core::*, Result};
    pub fn eye() -> Result<Mat> {
        Mat::eye(2, 3, CV_64F)?.to_mat()
    }
    pub fn rot(angle: f64) -> Result<Mat> {
        let cos = angle.cos();
        let sin = angle.sin();
        let m = Mat::from_slice_2d(&[[cos, sin, 0.0], [-sin, cos, 0.0]]);
        Ok(m?)
    }
    pub fn scale(s: f64) -> Result<Mat>{
        let q : MatExprResult<MatExpr> =eye()?*s;
        match q {
            MatExprResult::Ok(v) => return Ok(v.to_mat()?),
            MatExprResult::Err(e) => return Err(e),
        };
    }
}

#[allow(unused)]
impl MontageImage {
    fn new(file_name: &String, pos: &Point2i) -> MontageImage {
        let image = imgcodecs::imread(&file_name, 1).unwrap();
        // imgcodecs::conv
        if image.size().unwrap().width <= 0 {
            panic!("Can't open file"); // FIXME: show filename
        }

        MontageImage {
            aimage: AnnotationEditor::new(file_name),
            position: pos.clone(),
            image2: Mat::default(),
            dist_map: Mat::default(),
        }
    }
    fn render(&mut self, size: Size) -> Result<()> {
        self.image2 = Mat::zeros_size(size, CV_8UC3).unwrap().to_mat().unwrap();
        // //let size = self.image.size().unwrap();
        // // self.image2.set_rows(size.width);
        // let m2 = Mat::eye(2, 3, CV_64F)?;
        // let mut m3 = Mat2dOps::eye()?.t()?.to_mat()?;
        // m3 = (Mat2dOps::eye()?.t()? * Mat2dOps::rot(10f64)?).into_result()?.to_mat()?;

        let mut m = imgproc::get_rotation_matrix_2d(Point2f::new(0.0, 0.0), 10.0, 0.2)?;

        *m.at_2d_mut::<f64>(0, 2)?=self.position.x as f64;
        *m.at_2d_mut::<f64>(1, 2)?=self.position.y as f64;

        // let size= Size ::new(500,500);
        println!("s: {:?}", m);
        println!("p0: {:?}", m.at_2d::<f64>(0, 2)?);
        println!("p1: {:?}", m.at_2d::<f64>(1, 2)?);
        // println!("s: {:?}", m2.to_mat()?);

        imgproc::warp_affine(
            &self.aimage.image,
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

    fn dist(self: &MontageImage, p: &Point2i) -> f64 {
        let d = (self.position - *p).norm();
        d
    }
    fn sample(self: &MontageImage, p: &Point2i) -> Result<Vec3d> {
        let q = self.image2.at_2d::<Vec3b>(p.x, p.y)?;
        let a = q[0];
        // q[0] as f32 ,q[1] as f32,q[2] as f32
        let r = Vec3d::from([q[0] as f64, q[1] as f64, q[2] as f64]) / 255.;
        // let r=Point3d::new(1.,1.,1.);
        // let r=Point3f::new(q[0] as f32 ,q[1] as f32,q[2] as f32);
        // Ok(q.clone())
        Ok(r)
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
            width: 500,
            height: 500,
        };
        let mut image = Mat::zeros_size(size, CV_8UC3).unwrap().to_mat().unwrap();
        let images = vec![
            MontageImage::new(&String::from("r2.png"), &Point2i::new(300, 100)),
            MontageImage::new(&String::from("r3.png"), &Point2i::new(100, 300)),
            // MontageImage::new(&String::from("r4.png"), &Point2i::new(400, 400)),
        ];
        // let images = vec![Box::new(m)];
        let mut m = Montage {
            image: image,
            images: images,
            size: size,
        };
        m.render();
        m
    }
    fn render(&mut self) -> Result<()> {
        self.images
            .iter_mut()
            .for_each(|m| m.render(self.size).unwrap());

        self.image = Mat::new_size_with_default(self.size, CV_8UC3, Scalar::from(127))?;

        println!("ok?");
        let v0 = VecN::new(1, 2, 3, 4);
        let v = Vec3b::from([255, 0, 255]);
        for row in 0..self.size.height {
            for col in 0..self.size.width {
                let p: Point2i = Point_ { x: row, y: col };
                let mut dist_color: Vec<(f64, Vec3d)> = self
                    .images
                    .iter()
                    .map(|i| (i.dist(&p), i.sample(&p).unwrap()))
                    .collect();

                dist_color.sort_by(|(d, v), (d2, v2)| d.total_cmp(d2));

                let a = dist_color.get(0).unwrap();
                let b = dist_color.get(1).unwrap();
                let r = (b.1 * a.0 + a.1 * b.0) / (a.0 + b.0);

                let r2 = Vec3b::from([
                    (r[0] * 255.0) as u8,
                    (r[1] * 255.0) as u8,
                    (r[2] * 255.0) as u8,
                ]);
                *self.image.at_2d_mut::<Vec3b>(row, col)? = r2;
            }
        }
        println!("ok");

        if (false) {
            for i in self.images.iter_mut() {
                // i.render();
                let image = &i.aimage.image;
                let size = image.size().unwrap();
                let roi = Rect {
                    x: i.position.x,
                    y: i.position.y,
                    width: size.width,
                    height: size.height,
                };

                let mut dest = Mat::roi(&mut self.image, roi)?;
                println!("{:?}", dest);
                println!("{:?}", image);
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
