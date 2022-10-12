use std::sync::{Arc, Mutex};

use opencv::{
    core::*,
    highgui::{self, imshow, resize_window, EVENT_LBUTTONDOWN, WINDOW_GUI_EXPANDED},
    imgcodecs,
    imgproc::{self, convex_hull, LINE_8},
    prelude::*,
    types::VectorOfPoint,
    Result,
};
use serde::{Deserialize, Serialize};
use serde_yaml::{self};

#[allow(unused)]
#[derive(Debug, Serialize, Deserialize)]
struct OnDiskAnnotations {
    points: Vec<(i32, i32)>,
}

pub struct AnnotationEditor {
    points: VectorOfPoint, // = Arc::new(Mutex::new(VectorOfPoint::new()));
    annotation_file: String,
    #[allow(unused)]
    file_name: String,
    pub image: Mat,
}

#[allow(unused)]
impl AnnotationEditor {
    pub fn new(file_name: &String) -> AnnotationEditor {
        let mut image = imgcodecs::imread(&file_name, 1).unwrap();
        // FIXME: missing image not detected

        println!("asd {}", image.size().unwrap().width);

        let mut points = VectorOfPoint::new();

        let mut annotation_file = file_name.clone();
        annotation_file.push_str(".annot");

        if let Ok(f) = std::fs::File::open(&annotation_file) {
            let data: OnDiskAnnotations =
                serde_yaml::from_reader(f).expect("Could not read values.");
            for (x, y) in data.points {
                points.push(Point2i::new(x, y));
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
    fn save(self: &AnnotationEditor) {
        let f =
            std::fs::File::create(&self.annotation_file).expect("Can't open annot file for write");
        let a_points = self.points.iter().map(|p| (p.x, p.y)).collect();
        let ann: OnDiskAnnotations = OnDiskAnnotations { points: a_points };

        serde_yaml::to_writer(f, &ann).unwrap();
        dbg!("annot written");
    }
    fn draw(self: &AnnotationEditor) -> Result<Mat> {
        let mut frame = self.image.clone();
        let points = &self.points;
        let mut hull_points = VectorOfPoint::new();
        if points.len() > 0 {
            convex_hull(&points, &mut hull_points, true, true);

            for i in 1..points.len() {
                let color = Scalar::new(255., 0., 255., 0.);
                imgproc::line(
                    &mut frame,
                    points.get(i - 1)?,
                    points.get(i)?,
                    color,
                    2,
                    LINE_8,
                    0,
                )?;
            }
            for i in 0..hull_points.len() {
                let color = Scalar::new(0., 255., 0., 0.);
                let j = if i > 0 { i - 1 } else { hull_points.len() - 1 };
                imgproc::line(
                    &mut frame,
                    hull_points.get(j)?,
                    hull_points.get(i)?,
                    color,
                    2,
                    LINE_8,
                    0,
                )?;
            }
        }
        Ok(frame)
    }
    fn add_point(self: &mut AnnotationEditor, point: Point2i) {
        self.points.push(point);
    }

    pub(crate) fn make_dist_map(&self, m: Mat, size: Size_<i32>, pos: Point_<i32>) -> Result<Mat> {
        let mut dist_map = Mat::zeros_size(size, CV_64F)?.to_mat()?;
        fn map_point(m: &Mat, pos: &Point) -> Result<Point2i> {
            let p2 = Mat::from_slice(&[pos.x as f64, pos.y as f64, 1.0])?;
            let r = (m * p2).into_result()?.to_mat()?;
            let a: Point2i = Point2i::new(*r.at::<f64>(0)? as i32, *r.at::<f64>(1)? as i32);
            Ok(a)
        }

        let r = map_point(&m, &pos)?;

        for row in 0..size.height {
            for col in 0..size.width {
                let p1 = Point2i::new(col, row);
                let p2 = Point2i::new(col, row);
                let d = (p1 - pos).norm();
                // let p2 =Point2i::new(col+50, row+50);
                *dist_map.at_2d_mut::<f64>(row, col)? = (d - 100.0).max(0.1)
            }
        }
        Ok(dist_map)
    }
}

pub fn editor(file_name: &String) -> Result<()> {
    let annotation_editor = Arc::new(Mutex::new(AnnotationEditor::new(file_name)));
    let window = "montage-gen annotate";

    highgui::named_window(window, highgui::WINDOW_KEEPRATIO | WINDOW_GUI_EXPANDED)?;
    highgui::set_mouse_callback(
        window,
        Some(Box::new({
            let annotation_editor = Arc::clone(&annotation_editor);
            move |event, x, y, _flags| match event {
                EVENT_LBUTTONDOWN => {
                    println!(" {} {} ", x, y);
                    annotation_editor
                        .lock()
                        .unwrap()
                        .add_point(Point2i::new(x, y));
                }
                _ => {}
            }
        })),
    )?;

    resize_window(window, 500, 500)?;
    loop {
        let key = highgui::wait_key(100)?;
        if key == 27 {
            break;
        }
        let annotation_editor = annotation_editor.lock().unwrap();
        if key == 's' as i32 {
            annotation_editor.save();
            break;
        }
        let frame = annotation_editor.draw()?;
        imshow(window, &frame)?;
    }
    Ok(())
}
