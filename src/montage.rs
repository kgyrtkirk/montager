use std::sync::{Arc, Mutex};

use crate::annotate::AnnotationEditor;
use color_eyre::Result;
use opencv::highgui::{EVENT_LBUTTONUP, EVENT_MOUSEMOVE};
#[allow(unused)]
use opencv::{
    core::*,
    highgui::{self, imshow, resize_window, EVENT_LBUTTONDOWN, WINDOW_GUI_EXPANDED},
    imgcodecs,
    imgproc::{self, INTER_LINEAR, LINE_8},
    prelude::*,
    types::VectorOfPoint,
    // Result,
};

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

        let mut m = imgproc::get_rotation_matrix_2d(Point2f::new(0.0, 0.0), 0.0, 0.5)?;
        *m.at_2d_mut::<f64>(0, 2)? = self.position.x as f64;
        *m.at_2d_mut::<f64>(1, 2)? = self.position.y as f64;

        self.render_cache = Some(RenderedMontageImage {
            image: self.aimage.make_transformed_image(&m, size),
            dist_map: self.aimage.make_dist_map(&m, size)?,
        });

        Ok(Some({}))
    }

    fn dist(self: &MontageImage, p: &Point2i) -> f64 {
        self.render_cache
            .as_ref()
            .unwrap()
            .dist_map
            .at_2d::<f64>(p.y, p.x)
            .unwrap()
            .clone()
    }
    fn sample(self: &MontageImage, p: &Point2i) -> Result<Vec3d> {
        let q = self
            .render_cache
            .as_ref()
            .unwrap()
            .image
            .at_2d::<Vec3b>(p.y, p.x)?;
        let a = q[0];
        let r = Vec3d::from([q[0] as f64, q[1] as f64, q[2] as f64]) / 255.;
        Ok(r)
    }

    fn set_show_boundaries(&mut self, show_boundaries: bool) {
        self.aimage.set_show_boundaries(show_boundaries);
        self.render_cache = None;
    }
}

#[allow(unused)]
pub struct Montage {
    render_buffer: Option<Mat>,
    images: Vec<MontageImage>,
    size: Size2i,
    options: MontageOptions,
}

#[allow(unused)]
impl Montage {
    fn new(file_name: &String) -> Montage {
        let size = Size_ {
            width: 500,
            height: 500,
        };
        let images = vec![
            MontageImage::new(&String::from("r2.png"), &Point2i::new(300, 50)),
            MontageImage::new(&String::from("r3.png"), &Point2i::new(50, 150)),
            MontageImage::new(&String::from("r4.png"), &Point2i::new(300, 250)),
        ];

        // let images = vec![Box::new(m)];
        let mut m = Montage {
            render_buffer: None,
            images: images,
            size: size,
            options: Default::default(),
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
            self.render_buffer = None;
        }
        if self.render_buffer.is_some() {
            return Ok({});
        }

        let mut selfimage = Mat::new_size_with_default(self.size, CV_8UC3, Scalar::from(127))?;

        let v0 = VecN::new(1, 2, 3, 4);
        let v = Vec3b::from([255, 0, 255]);
        for row in 0..self.size.height {
            for col in 0..self.size.width {
                let p: Point2i = Point_ { x: col, y: row };
                let mut dist_color: Vec<(f64, Vec3d)> = self
                    .images
                    .iter()
                    .map(|i| (i.dist(&p), i.sample(&p).unwrap()))
                    .collect();

                dist_color.sort_by(|(d, v), (d2, v2)| d.total_cmp(d2));

                let a = dist_color.get(0).unwrap();
                let b = dist_color.get(1).unwrap();

                // let r = if a.0 < 100.0 { a.1 } else { b.1 };//* a.0);// + a.1 * b.0) / (a.0 + b.0+1.0);
                // let r = (b.1 * a.0 + a.1 * b.0) / (a.0 + b.0+1.0);
                let r = (b.1 * a.0 + a.1 * b.0) / (a.0 + b.0);

                let r2 = Vec3b::from([
                    (r[0] * 255.0) as u8,
                    (r[1] * 255.0) as u8,
                    (r[2] * 255.0) as u8,
                ]);
                *selfimage.at_2d_mut::<Vec3b>(row, col)? = r2;
            }
        }

        self.render_buffer = Some(selfimage);

        Ok({})
    }

    fn toggle_show_boundaries(&mut self) {
        self.options.show_boundaries = !self.options.show_boundaries;
        for i in &mut self.images {
            i.set_show_boundaries(self.options.show_boundaries);
        }
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
            self.last_pos = *pos;
        }
    }
}

#[derive(Default)]
pub struct MontageOptions {
    pub show_boundaries: bool,
}

struct MontageEditor {
    montage: Montage,
    // FIXME: figure out what +Send means
    mod_state: Option<Box<dyn Modification + Send>>,
}

enum MouseEvent {
    Move,
    LButtonDown,
    LButtonUp,
}

#[allow(unused)]
impl MontageEditor {
    fn new(file_name: &String) -> MontageEditor {
        let montage = Montage::new(file_name);
        MontageEditor {
            montage: montage,
            mod_state: None,
        }
    }
    fn mouse_event(&mut self, event: MouseEvent, pos: &Point2i) {
        if let Some(m) = self.mod_state.as_mut() {
            m.as_mut().apply(pos, &mut self.montage);
        }

        match (event) {
            MouseEvent::Move => {}
            MouseEvent::LButtonDown => {
                self.mod_state = Some(Box::new(MoveModification {
                    last_pos: *pos,
                    image_idx: 0,
                }))
            }
            MouseEvent::LButtonUp => self.mod_state = None,
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
        let key = highgui::wait_key(50)?;
        if key == 27 {
            break;
        }
        let mut editor = montage_editor.lock().unwrap();

        if key == 'e' as i32 {
            // FIXME: ideally this should be emitting an event automatically
            editor.montage.toggle_show_boundaries();
            println!("toggle-boundaries");
        }
        editor.montage.render()?;
        imshow(window, &editor.montage.render_buffer.as_ref().unwrap())?;
    }
    Ok(())
}
