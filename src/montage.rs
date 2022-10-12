use std::{
    sync::{Arc, Mutex},
};

use opencv::highgui::{EVENT_LBUTTONUP, EVENT_MOUSEMOVE};
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
struct RenderedMontageImage {
    image: Mat,
    dist_map: Mat,
}

#[allow(unused)]
struct MontageImage {
    aimage: AnnotationEditor,
    position: Point2i,
    render_cache: Option<RenderedMontageImage>,
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
            render_cache: None,
        }
    }
    fn move1(&mut self, p: &Point2i) {
        self.position += *p;
        self.render_cache = None;
    }
    fn update(&mut self, size: Size) -> Result<Option<()>> {
        if self.render_cache.is_some() {
            return Ok(None);
        }

        let mut image = Mat::default();

        let mut m = imgproc::get_rotation_matrix_2d(Point2f::new(0.0, 0.0), 10.0, 0.2)?;

        *m.at_2d_mut::<f64>(0, 2)? = self.position.x as f64;
        *m.at_2d_mut::<f64>(1, 2)? = self.position.y as f64;

        imgproc::warp_affine(
            &self.aimage.image,
            &mut image,
            &m,
            size,
            INTER_LINEAR,
            BORDER_CONSTANT,
            Scalar::new(0.0, 0.0, 0.0, 0.0),
        );

        let pt1 = Point2i::new(0, 0);
        let pt2 = self.position;
        let pt3 = Point2i::new(size.width, size.height);
        let color = Scalar::new(0., 255., 255., 255.);
        imgproc::line(&mut image, pt1, pt2, color, 8, LINE_8, 0);
        imgproc::line(&mut image, pt2, pt3, color, 8, LINE_8, 0);
        self.render_cache = Some(RenderedMontageImage {
            image: image,
            dist_map: Mat::default(),
        });

        Ok(Some({}))
    }

    fn dist(self: &MontageImage, p: &Point2i) -> f64 {
        let d = (self.position - *p).norm();
        d
    }
    fn sample(self: &MontageImage, p: &Point2i) -> Result<Vec3d> {
        let q = self
            .render_cache
            .as_ref()
            .unwrap()
            .image
            .at_2d::<Vec3b>(p.x, p.y)?;
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
pub struct Montage {
    image1: Option<Mat>,
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
            MontageImage::new(&String::from("r2.png"), &Point2i::new(400, 100)),
            MontageImage::new(&String::from("r3.png"), &Point2i::new(100, 300)),
            // MontageImage::new(&String::from("r4.png"), &Point2i::new(400, 400)),
        ];
        let mut p = images.get(0).unwrap();
        let mut p = images.get(0).unwrap().position;

        // let images = vec![Box::new(m)];
        let mut m = Montage {
            image1: None,
            images: images,
            size: size,
        };
        m.render();
        m
    }
    fn render(&mut self) -> Result<()> {
        let mut changes = false;
        // how about an event instead?
        let mod_count: i32 = self
            .images
            .iter_mut()
            .map(|m| m.update(self.size).unwrap().is_some() as i32)
            .sum();
        if mod_count > 0 {
            self.image1 = None;
        }
        if self.image1.is_some() {
            return Ok({});
        }

        let mut selfimage = Mat::new_size_with_default(self.size, CV_8UC3, Scalar::from(127))?;

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
                *selfimage.at_2d_mut::<Vec3b>(row, col)? = r2;
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

                let mut dest = Mat::roi(&mut selfimage, roi)?;
                println!("{:?}", dest);
                println!("{:?}", image);
                //           i.image2.copy_to(&mut dest)?;
                image.copy_to(&mut dest)?;
                // normalize(src, dst, alpha, beta, norm_type, dtype, mask)
                // panic!("asd");
                // self.image.re
            }
        }
        self.image1 = Some(selfimage);

        Ok({})
    }
}

struct MoveModification {
    last_pos: Point2i,
    image_idx: usize,
}

pub trait Modification {
    fn apply(&mut self, pos: &Point2i, montage: &mut Montage);
}

impl Modification for MoveModification {
    fn apply(&mut self, pos: &Point2i, montage: &mut Montage) {
        let delta = *pos - self.last_pos;
        if delta.norm() > 0. {
            let img = (montage.images).get_mut(self.image_idx).unwrap();
            img.move1(&delta);
            self.last_pos=*pos;
        }
    }
}

struct MontageEditor {
    montage: Montage,
    // FIXME: figure out what +Send means
    modState: Option<Box<dyn Modification + Send>>,
    //    active_image : i32,
}

enum MouseEvent {
    Move,
    LButtonDown,
    LButtonUp,
}

impl MouseEvent {
    pub fn isUp(&self) -> bool {
        match self {
            MouseEvent::LButtonUp => true,
            _ => false,
        }
    }
}

#[allow(unused)]
impl MontageEditor {
    fn new(file_name: &String) -> MontageEditor {
        let montage = Montage::new(file_name);
        MontageEditor {
            montage: montage,
            modState: None,
        }
    }
    fn mouse_event(&mut self, event: MouseEvent, pos: &Point2i) {
        if let Some(m) = self.modState.as_mut() {
            m.as_mut().apply(pos, &mut self.montage);
        }

        match (event) {
            MouseEvent::Move => {}
            MouseEvent::LButtonDown => {
                self.modState = Some(Box::new(MoveModification {
                    last_pos: *pos,
                    image_idx: 0,
                }))
            }
            MouseEvent::LButtonUp => self.modState = None,
        }
    }
}

pub fn editor(file_name: &Vec<String>) -> Result<()> {
    let montage_editor: Arc<Mutex<MontageEditor>> =
        Arc::new(Mutex::new(MontageEditor::new(file_name.get(0).unwrap())));

    let window = "montage-gen montage";

    highgui::named_window(window, highgui::WINDOW_KEEPRATIO | WINDOW_GUI_EXPANDED)?;

    highgui::set_mouse_callback(
        window,
        Some(Box::new({
            let montage_editor = Arc::clone(&montage_editor);
            move |event, x, y, _flags| {
                let p = Point2i::new(x, y);
                let mut montage_editor = montage_editor.lock().unwrap();

                match event {
                    EVENT_MOUSEMOVE => {
                        montage_editor.mouse_event(MouseEvent::Move, &p);
                    }
                    EVENT_LBUTTONDOWN => {
                        montage_editor.mouse_event(MouseEvent::LButtonDown, &p);
                    }
                    EVENT_LBUTTONUP => {
                        montage_editor.mouse_event(MouseEvent::LButtonUp, &p);
                    }
                    _ => {}
                }
            }
        })),
    )?;

    resize_window(window, 500, 500)?;
    loop {
        let key = highgui::wait_key(10)?;
        if key == 27 {
            break;
        }
        let mut editor = montage_editor.lock().unwrap();
        editor.montage.render()?;
        imshow(window, &editor.montage.image1.as_ref().unwrap())?;
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
